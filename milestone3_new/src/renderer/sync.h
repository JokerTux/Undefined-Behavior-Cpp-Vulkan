#pragma once
#include "vk_utils.h"

class Command_pool;

enum class DrawResult{
	Done,
	NeedRecreate,
	FatalError
};

class Create_sync{
	public:
		bool create_sync_obj(VkContext* vkcontext);
		DrawResult draw_frame(VkContext* vkcontext, Command_pool& command_pool);
		~Create_sync();
		
	private:
		VkContext* des_context = nullptr;
};