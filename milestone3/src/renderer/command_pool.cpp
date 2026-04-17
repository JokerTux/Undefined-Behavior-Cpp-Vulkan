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
	vkcontext->commandBuffers.resize(vkcontext->MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vkcontext->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(vkcontext->commandBuffers.size());

	VK_CHECK(vkAllocateCommandBuffers(vkcontext->device, &allocInfo, vkcontext->commandBuffers.data()));
	return true;
}

bool Command_pool::record_command_buff(VkContext* vkcontext, uint32_t currentFrame, uint32_t imageIndex){
	VkCommandBuffer cmd = vkcontext->commandBuffers[currentFrame];

	VK_CHECK(vkResetCommandBuffer(cmd, 0));
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
	
	VkClearValue clearColor = {{{0.0f, 0.1f, 0.1f, 1.0f}}};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vkcontext->render_pass;
	renderPassInfo.framebuffer = vkcontext->framebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = vkcontext->swapchainExtent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vkcontext->graphicsPipeline);
	vkCmdDraw(cmd, 3, 1, 0, 0);
	vkCmdEndRenderPass(cmd);
	VK_CHECK(vkEndCommandBuffer(cmd));
	
	return true;
}

Command_pool::~Command_pool(){
	if(des_context && des_context->device != VK_NULL_HANDLE && des_context->commandPool != VK_NULL_HANDLE){
		vkDestroyCommandPool(des_context->device, des_context->commandPool, nullptr);
		des_context->commandPool = VK_NULL_HANDLE;
	}
}