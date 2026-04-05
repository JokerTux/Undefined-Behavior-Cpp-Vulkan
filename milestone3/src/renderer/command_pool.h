#pragma once

#include "vk_utils.h"


class Command_pool{
	public:
		bool create_command_pool(VkContext* vkcontext);
		bool create_command_buffers(VkContext* vkcontext);
		bool record_command_buff(VkContext* vkcontext);
		~Command_pool();
	private:
		VkContext* des_context = VK_NULL_HANDLE;
};