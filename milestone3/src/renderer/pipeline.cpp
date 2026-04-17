#include "pipeline.h"
#include <fstream>
#include <vector>

static std::vector<char> read_b_shader(const std::string& file_path){
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);

	if(!file.is_open()){
		std::cerr << "Failed to read the shader : " << file_path << std::endl;
		file.close();
		return {};
	}

	size_t file_size = static_cast<size_t>(file.tellg());
	std::vector<char> b_buffer(file_size);

	file.seekg(0);
	file.read(b_buffer.data(), file_size);
	file.close();

	return b_buffer;
}

static VkShaderModule create_shader_module(VkContext* vkcontext, const std::vector<char>& b_code){
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = b_code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(b_code.data());

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	VkResult r = vkCreateShaderModule(vkcontext->device, &createInfo, nullptr, &shaderModule);
	if(r != VK_SUCCESS){
		std::cerr << "vkCreateShaderModule failed : " << r << std::endl;
		return VK_NULL_HANDLE;
	}
	
	return shaderModule;
}

bool Graphics_pipeline::create_pipeline(VkContext* vkcontext){
	des_context = vkcontext;

	auto vertShaderCode = read_b_shader("src/shaders/triangle.vert.spv");
	auto fragShaderCode = read_b_shader("src/shaders/triangle.frag.spv");

	if(vertShaderCode.empty() || fragShaderCode.empty()){
		std::cerr << "Error while loading the shader binary code" << std::endl;
		return false;
	}

	VkShaderModule vertShaderModule = create_shader_module(vkcontext, vertShaderCode);
	VkShaderModule fragShaderModule = create_shader_module(vkcontext, fragShaderCode);

	if(vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE){
		std::cerr << "Error while loading the shader modules" << std::endl;
		return false;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(vkcontext->swapchainExtent.width);
	viewport.height = static_cast<float>(vkcontext->swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = vkcontext->swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; // simplest for now
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if(vkCreatePipelineLayout(vkcontext->device, &pipelineLayoutInfo, nullptr, &vkcontext->pipelineLayout) != VK_SUCCESS){
		std::cerr << "vkCreatePipelineLayout failed" << std::endl;
		vkDestroyShaderModule(vkcontext->device, vertShaderModule, nullptr);
		vkDestroyShaderModule(vkcontext->device, fragShaderModule, nullptr);

		return false;
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = vkcontext->pipelineLayout;
	pipelineInfo.renderPass = vkcontext->render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if(vkCreateGraphicsPipelines(vkcontext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkcontext->graphicsPipeline) != VK_SUCCESS){
		std::cerr << "vkCreateGraphicsPipelines failed" << std::endl;
		vkDestroyPipelineLayout(vkcontext->device, vkcontext->pipelineLayout, nullptr);
		vkcontext->pipelineLayout = VK_NULL_HANDLE;
		vkDestroyShaderModule(vkcontext->device, vertShaderModule, nullptr);
		vkDestroyShaderModule(vkcontext->device, fragShaderModule, nullptr);

		return false;
	}
	std::cout << "pipeline created" << std::endl;
	vkDestroyShaderModule(vkcontext->device, vertShaderModule, nullptr);
	vkDestroyShaderModule(vkcontext->device, fragShaderModule, nullptr);

	return true;
	}

Graphics_pipeline::~Graphics_pipeline(){
	if(des_context && des_context->device != VK_NULL_HANDLE){
		if(des_context->graphicsPipeline != VK_NULL_HANDLE){
			vkDestroyPipeline(des_context->device, des_context->graphicsPipeline, nullptr);
			des_context->graphicsPipeline = VK_NULL_HANDLE;
		}

		if(des_context->pipelineLayout != VK_NULL_HANDLE){
			vkDestroyPipelineLayout(des_context->device, des_context->pipelineLayout, nullptr);
			des_context->pipelineLayout = VK_NULL_HANDLE;
		}
	}
}
