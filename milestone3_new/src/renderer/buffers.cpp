#include "buffers.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <iostream>
#include <stdexcept>
 
// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
 
// Finds a memory type that satisfies both the hardware type-filter bitmask
// and the property flags we need (e.g. HOST_VISIBLE for staging,
// DEVICE_LOCAL for the final buffers).
static uint32_t find_memory_type(VkPhysicalDevice gpu, uint32_t typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(gpu, &memProps);
 
    for(uint32_t i = 0; i < memProps.memoryTypeCount; i++){
        bool typeMatch = (typeFilter & (1u << i)) != 0;
        bool propMatch = (memProps.memoryTypes[i].propertyFlags & properties) == properties;
        if (typeMatch && propMatch)
            return i;
    }
    throw std::runtime_error("find_memory_type: no suitable memory type found");
}
 
// Allocates a VkBuffer with its backing VkDeviceMemory.
static bool create_buffer(VkContext* vkcontext, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& outBuffer, VkDeviceMemory& outMemory){
    VkBufferCreateInfo bufInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufInfo.size = size;
    bufInfo.usage = usage;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
 
    VK_CHECK(vkCreateBuffer(vkcontext->device, &bufInfo, nullptr, &outBuffer));
 
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(vkcontext->device, outBuffer, &memReqs);
 
    VkMemoryAllocateInfo allocInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = find_memory_type(vkcontext->gpu, memReqs.memoryTypeBits, properties);
 
    if(vkAllocateMemory(vkcontext->device, &allocInfo, nullptr, &outMemory) != VK_SUCCESS){
        std::cerr << "create_buffer: vkAllocateMemory failed\n";
        vkDestroyBuffer(vkcontext->device, outBuffer, nullptr);
        outBuffer = VK_NULL_HANDLE;
        return false;
    }
 
    vkBindBufferMemory(vkcontext->device, outBuffer, outMemory, 0);
    return true;
}
 
// Copies size bytes from src to dst using a one-shot command buffer.
static bool copy_buffer(VkContext* vkcontext, VkBuffer src, VkBuffer dst, VkDeviceSize size){
    // Allocate a temporary command buffer from the existing pool.
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vkcontext->commandPool;
    allocInfo.commandBufferCount = 1;
 
    VkCommandBuffer cmd;
    if(vkAllocateCommandBuffers(vkcontext->device, &allocInfo, &cmd) != VK_SUCCESS){
        std::cerr << "copy_buffer: vkAllocateCommandBuffers failed\n";
        return false;
    }
 
    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);
 
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &copyRegion);
 
    vkEndCommandBuffer(cmd);
 
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
 
    vkQueueSubmit(vkcontext->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkcontext->graphicsQueue);
 
    vkFreeCommandBuffers(vkcontext->device, vkcontext->commandPool, 1, &cmd);
    return true;
}
 
// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
 
bool create_mesh_buffers(VkContext* vkcontext, const Mesh& mesh)
{
    // ---- Vertex buffer ----
    {
        VkDeviceSize size = sizeof(Vertex) * mesh.vertices.size();
 
        // Staging buffer: CPU-visible so we can memcpy into it.
        VkBuffer stagingBuf;
        VkDeviceMemory stagingMem;
        if(!create_buffer(vkcontext, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuf, stagingMem)){
            return false;
        }
 
        void* data;
        vkMapMemory(vkcontext->device, stagingMem, 0, size, 0, &data);
        std::memcpy(data, mesh.vertices.data(), static_cast<size_t>(size));
        vkUnmapMemory(vkcontext->device, stagingMem);
 
        // Device-local buffer: GPU-only, fastest access in shaders.
        if(!create_buffer(vkcontext, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkcontext->vertexBuffer, vkcontext->vertexMemory)){
            vkDestroyBuffer(vkcontext->device, stagingBuf, nullptr);
            vkFreeMemory(vkcontext->device, stagingMem, nullptr);
            return false;
        }
 
        if(!copy_buffer(vkcontext, stagingBuf, vkcontext->vertexBuffer, size)){
            vkDestroyBuffer(vkcontext->device, stagingBuf, nullptr);
            vkFreeMemory(vkcontext->device, stagingMem, nullptr);
            return false;
        }
 
        vkDestroyBuffer(vkcontext->device, stagingBuf, nullptr);
        vkFreeMemory(vkcontext->device, stagingMem, nullptr);
    }
 
    // ---- Index buffer ----
    {
        VkDeviceSize size = sizeof(uint32_t) * mesh.indices.size();
 
        VkBuffer stagingBuf;
        VkDeviceMemory stagingMem;
        if(!create_buffer(vkcontext, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuf, stagingMem)){
            return false;
        }
 
        void* data;
        vkMapMemory(vkcontext->device, stagingMem, 0, size, 0, &data);
        std::memcpy(data, mesh.indices.data(), static_cast<size_t>(size));
        vkUnmapMemory(vkcontext->device, stagingMem);
 
        if(!create_buffer(vkcontext, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkcontext->indexBuffer, vkcontext->indexMemory)){
            vkDestroyBuffer(vkcontext->device, stagingBuf, nullptr);
            vkFreeMemory(vkcontext->device, stagingMem, nullptr);
            return false;
        }
 
        if(!copy_buffer(vkcontext, stagingBuf, vkcontext->indexBuffer, size)){
            vkDestroyBuffer(vkcontext->device, stagingBuf, nullptr);
            vkFreeMemory(vkcontext->device, stagingMem, nullptr);
            return false;
        }
 
        vkDestroyBuffer(vkcontext->device, stagingBuf, nullptr);
        vkFreeMemory(vkcontext->device, stagingMem, nullptr);
    }
 
    vkcontext->indexCount = static_cast<uint32_t>(mesh.indices.size());
    std::cout << "Mesh buffers created : " << mesh.vertices.size() << " vertices, " << mesh.indices.size()  << " indices\n";
    return true;
}

bool create_ubo(VkContext* vkcontext){
    VkDeviceSize size = sizeof(UBOData);

    if(!create_buffer(vkcontext, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vkcontext->uboBuffer, vkcontext->uboMemory)){
        return false;
    }

    // Persistent map — we update this every frame, no need to map/unmap each time
    vkMapMemory(vkcontext->device, vkcontext->uboMemory, 0, size, 0, &vkcontext->uboMapped);

    // --- Descriptor set layout ---
    VkDescriptorSetLayoutBinding binding{};
    binding.binding         = 0;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings    = &binding;
    VK_CHECK(vkCreateDescriptorSetLayout(vkcontext->device, &layoutInfo, nullptr, &vkcontext->descriptorSetLayout));

    // --- Descriptor pool ---
    VkDescriptorPoolSize poolSize{};
    poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.maxSets       = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    VK_CHECK(vkCreateDescriptorPool(vkcontext->device, &poolInfo, nullptr, &vkcontext->descriptorPool));

    // --- Descriptor set ---
    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool     = vkcontext->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &vkcontext->descriptorSetLayout;
    VK_CHECK(vkAllocateDescriptorSets(vkcontext->device, &allocInfo, &vkcontext->descriptorSet));

    // Point the descriptor at the UBO buffer
    VkDescriptorBufferInfo bufInfo{};
    bufInfo.buffer = vkcontext->uboBuffer;
    bufInfo.offset = 0;
    bufInfo.range  = sizeof(UBOData);

    VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstSet          = vkcontext->descriptorSet;
    write.dstBinding      = 0;
    write.descriptorCount = 1;
    write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo     = &bufInfo;
    vkUpdateDescriptorSets(vkcontext->device, 1, &write, 0, nullptr);

    return true;
}

void update_ubo(VkContext* vkcontext, float aspectRatio, float dt){

    static float angle = 0.0f;
    angle += (dt / 10) * glm::radians(90.0f); // 90 degrees per second

    UBOData ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));   // identity — mesh stays at origin
    ubo.view  = glm::lookAt(
        glm::vec3(0.0f, 0.0f,   9.0f),  // camera position — tweak these!
        glm::vec3(0.0f, 0.0f,   0.0f),     // looking at origin
        glm::vec3(0.0f, 1.0f,   0.0f)      // up
    );
    ubo.proj  = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100000.0f);
    ubo.proj[1][1] *= -1;  // GLM is OpenGL-convention, Vulkan Y is flipped

    memcpy(vkcontext->uboMapped, &ubo, sizeof(ubo));
}

void destroy_ubo(VkContext* vkcontext){
    if(!vkcontext || vkcontext->device == VK_NULL_HANDLE) return;
    if(vkcontext->descriptorPool != VK_NULL_HANDLE){
        vkDestroyDescriptorPool(vkcontext->device, vkcontext->descriptorPool, nullptr);
        vkcontext->descriptorPool = VK_NULL_HANDLE;
    }
    if(vkcontext->descriptorSetLayout != VK_NULL_HANDLE){
        vkDestroyDescriptorSetLayout(vkcontext->device, vkcontext->descriptorSetLayout, nullptr);
        vkcontext->descriptorSetLayout = VK_NULL_HANDLE;
    }
    if(vkcontext->uboBuffer != VK_NULL_HANDLE){
        vkUnmapMemory(vkcontext->device, vkcontext->uboMemory);
        vkDestroyBuffer(vkcontext->device, vkcontext->uboBuffer, nullptr);
        vkFreeMemory(vkcontext->device, vkcontext->uboMemory, nullptr);
        vkcontext->uboBuffer = VK_NULL_HANDLE;
        vkcontext->uboMemory = VK_NULL_HANDLE;
    }
}
 
void destroy_mesh_buffers(VkContext* vkcontext){
    if(!vkcontext || vkcontext->device == VK_NULL_HANDLE) return;
 
    if(vkcontext->indexBuffer != VK_NULL_HANDLE){
        vkDestroyBuffer(vkcontext->device, vkcontext->indexBuffer, nullptr);
        vkFreeMemory(vkcontext->device, vkcontext->indexMemory, nullptr);
        vkcontext->indexBuffer = VK_NULL_HANDLE;
        vkcontext->indexMemory = VK_NULL_HANDLE;
    }
    
    if(vkcontext->vertexBuffer != VK_NULL_HANDLE){
        vkDestroyBuffer(vkcontext->device, vkcontext->vertexBuffer, nullptr);
        vkFreeMemory(vkcontext->device, vkcontext->vertexMemory, nullptr);
        vkcontext->vertexBuffer = VK_NULL_HANDLE;
        vkcontext->vertexMemory = VK_NULL_HANDLE;
    }
}