#pragma once

#include <vulkan/vulkan.h>
#include "vk_utils.h"


class Select_device{
	public:
		bool pick_physical_device(VkContext* vkcontext);
		bool create_logical_device(VkContext* vkcontext);
		~Select_device();

	private:
		VkContext* des_context = nullptr;
};