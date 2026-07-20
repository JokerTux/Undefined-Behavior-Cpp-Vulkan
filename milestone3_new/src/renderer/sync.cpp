#include <iostream>
#include "sync.h"
#include "command_pool.h"

bool Create_sync::create_sync_obj(VkContext* vkcontext){
	des_context = vkcontext;

	vkcontext->imageAvailableSemaphore.resize(vkcontext->MAX_FRAMES_IN_FLIGHT);
	vkcontext->renderFinishedSemaphore.resize(vkcontext->swapchainImages.size());
	vkcontext->inFlightFence.resize(vkcontext->MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(uint32_t i = 0; i < vkcontext->MAX_FRAMES_IN_FLIGHT; i++){
		VK_CHECK(vkCreateSemaphore(vkcontext->device, &semaphoreInfo, nullptr, &vkcontext->imageAvailableSemaphore[i]));
		VK_CHECK(vkCreateFence(vkcontext->device, &fenceInfo, nullptr, &vkcontext->inFlightFence[i]));
	}

	for(uint32_t i = 0; i < vkcontext->swapchainImages.size(); i++){
		VK_CHECK(vkCreateSemaphore(vkcontext->device, &semaphoreInfo, nullptr, &vkcontext->renderFinishedSemaphore[i]));
	}

	return true;
}

DrawResult Create_sync::draw_frame(VkContext* vkcontext, Command_pool& command_pool){
	//BAD CODE WARNING :
	VkResult wait_fences_r = vkWaitForFences(vkcontext->device, 1, &vkcontext->inFlightFence[vkcontext->currentFrame], VK_TRUE, UINT64_MAX);
	if(wait_fences_r != VK_SUCCESS){
		std::cerr << "vkWaitForFences failed: " << wait_fences_r << std::endl;
		return DrawResult::FatalError;
	}
	
	uint32_t imageIndex = 0;
	VkResult acquireResult = vkAcquireNextImageKHR(vkcontext->device, vkcontext->swapchain, UINT64_MAX, vkcontext->imageAvailableSemaphore[vkcontext->currentFrame], VK_NULL_HANDLE, &imageIndex);

	if(acquireResult == VK_ERROR_OUT_OF_DATE_KHR){
		return DrawResult::NeedRecreate;
	}

	if(acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR){
		std::cerr << "vkAcquireNextImageKHR failed: " << acquireResult << std::endl;
		return DrawResult::FatalError;
	}

	if(!command_pool.record_command_buff(vkcontext, vkcontext->currentFrame, imageIndex)){
           std::cerr << "create_command_buffers failed" << std::endl;
           return DrawResult::FatalError;
    }

	VkResult res_fence_r = vkResetFences(vkcontext->device, 1, &vkcontext->inFlightFence[vkcontext->currentFrame]);
	if(res_fence_r != VK_SUCCESS){
		std::cerr << "vkResetFences failed: " << res_fence_r << std::endl;
		return DrawResult::FatalError;
	}

	//TODO :

	VkSemaphoreSubmitInfo waitInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
	waitInfo.semaphore = vkcontext->imageAvailableSemaphore[vkcontext->currentFrame];
	waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSemaphoreSubmitInfo signalInfo{VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO};
	signalInfo.semaphore = vkcontext->renderFinishedSemaphore[imageIndex];
	signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

	VkCommandBufferSubmitInfo cmdInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO};
	cmdInfo.commandBuffer = vkcontext->commandBuffers[vkcontext->currentFrame];

	VkSubmitInfo2 submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
	submitInfo.waitSemaphoreInfoCount = 1;
	submitInfo.pWaitSemaphoreInfos = &waitInfo;
	submitInfo.commandBufferInfoCount = 1;
	submitInfo.pCommandBufferInfos = &cmdInfo;
	submitInfo.signalSemaphoreInfoCount = 1;
	submitInfo.pSignalSemaphoreInfos = &signalInfo;

	VkResult queue_r = vkQueueSubmit2(vkcontext->graphicsQueue, 1, &submitInfo, vkcontext->inFlightFence[vkcontext->currentFrame]);
	if(queue_r != VK_SUCCESS){
		std::cerr << "vkQueueSubmit2 failed: " << queue_r << std::endl;
		return DrawResult::FatalError;
	}

	VkSemaphore presentWait = vkcontext->renderFinishedSemaphore[imageIndex];

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentWait;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vkcontext->swapchain;
	presentInfo.pImageIndices = &imageIndex;

	VkResult presentResult = vkQueuePresentKHR(vkcontext->presentQueue, &presentInfo);

	if(presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR){
		return DrawResult::NeedRecreate;
	}

	if(presentResult != VK_SUCCESS){
		std::cerr << "vkQueuePresentKHR failed: " << presentResult << std::endl;
		return DrawResult::FatalError;
	}
	
	vkcontext->currentFrame = (vkcontext->currentFrame + 1) % vkcontext->MAX_FRAMES_IN_FLIGHT;
	//std::cout << "Frame : " << vkcontext->currentFrame << std::endl;

	return DrawResult::Done;
}


Create_sync::~Create_sync(){
	if(!des_context || des_context->device == VK_NULL_HANDLE){
		return;
	}

	for(uint32_t i = 0; i < des_context->MAX_FRAMES_IN_FLIGHT; i++){
		vkDestroySemaphore(des_context->device, des_context->imageAvailableSemaphore[i], nullptr);
		vkDestroyFence(des_context->device, des_context->inFlightFence[i], nullptr);
	}
	
	for(uint32_t i = 0; i < des_context->swapchainImages.size(); i++){
		vkDestroySemaphore(des_context->device, des_context->renderFinishedSemaphore[i], nullptr);
	}
}