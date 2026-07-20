#pragma once

#include "vk_utils.h"


class Command_pool{
	public:
		bool create_command_pool(VkContext* vkcontext);
		bool create_command_buffers(VkContext* vkcontext);
		bool record_command_buff(VkContext* vkcontext, uint32_t currentFrame, uint32_t imageIndex);
		~Command_pool();
	private:
		VkContext* des_context = nullptr;
};