#pragma once

#include "vk_utils.h"


class Create_render_pass{
	public:
		bool create_ren_pass(VkContext* vkcontext);
		~Create_render_pass();

	private:
		VkContext* des_context = nullptr;
};