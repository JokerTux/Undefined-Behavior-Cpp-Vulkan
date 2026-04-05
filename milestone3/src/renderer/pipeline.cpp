#include "pipeline.h"
#include <fstream>
#include <vector>

static std::vector<char> read_b_shader(const std::string& file_path){
	std::ifstream file(file_path, std::ios::ate | std::ios::binary);

	if(!file.is_open()){
		std::cerr << "Failed to read the shader : " << file_path << std::endl;
		file.close();
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

	VkShaderModule shaderModule;
	VkResult r = vkCreateShaderModule(vkcontext->device, &createInfo, nullptr, &shaderModule);
	if(r != VK_SUCCESS){
		std::cerr << "vkCreateShaderModule failed : " << r << std::endl;
		return VK_NULL_HANDLE;
	}
	
	return shaderModule;
}