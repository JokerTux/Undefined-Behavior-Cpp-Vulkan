#include "command_pool.h"


bool Command_pool::create_command_pool(VkContext* vkcontext){
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = vkcontext->graphicsIndex;

	VK_CHECK(vkCreateCommandPool(vkcontext->device, &poolInfo, nullptr, &vkcontext->commandPool));
	
	return true;
}

bool Command_pool::create_command_buffers(VkContext* vkcontext){
	des_context = vkcontext;
	vkcontext->commandBuffers.resize(vkcontext->framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vkcontext->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(vkcontext->commandBuffers.size());

	VK_CHECK(vkAllocateCommandBuffers(vkcontext->device, &allocInfo, vkcontext->commandBuffers.data()));
	return true;
}

bool Command_pool::record_command_buff(VkContext* vkcontext){
	for(size_t i = 0; i < vkcontext->commandBuffers.size(); i++){
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		VK_CHECK(vkBeginCommandBuffer(vkcontext->commandBuffers[i], &beginInfo));
	
		VkClearValue clearColor = {{{1.0f, 0.1f, 0.1f, 1.0f}}};

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vkcontext->render_pass;
		renderPassInfo.framebuffer = vkcontext->framebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = vkcontext->swapchainExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(vkcontext->commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(vkcontext->commandBuffers[i]);

		VK_CHECK(vkEndCommandBuffer(vkcontext->commandBuffers[i]));
	}
	return true;
}

Command_pool::~Command_pool(){
	if(des_context && des_context->device != VK_NULL_HANDLE && des_context->commandPool != VK_NULL_HANDLE){
		vkDestroyCommandPool(des_context->device, des_context->commandPool, nullptr);
		des_context->commandPool = VK_NULL_HANDLE;
	}
}