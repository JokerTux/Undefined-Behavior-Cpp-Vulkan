#include "render_pass.h"


bool Create_render_pass::create_ren_pass(VkContext* vkcontext){
	des_context = vkcontext;
	VkAttachmentDescription color_attachments{};
	color_attachments.format = vkcontext->surfaceFormat.format;
	color_attachments.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachments.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachments.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachments.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachments;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(vkcontext->device, &render_pass_info, nullptr, &vkcontext->render_pass));
	return true;
}

Create_render_pass::~Create_render_pass(){
	if(des_context && des_context != VK_NULL_HANDLE){
		if(des_context->device != VK_NULL_HANDLE){
			vkDestroyRenderPass(des_context->device, des_context->render_pass, nullptr);
			des_context-> render_pass = VK_NULL_HANDLE;
		}
	}
}