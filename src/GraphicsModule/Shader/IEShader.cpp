/* Include this file's header. */
#include "IEShader.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"


/* Include dependencies from Core. */
#include "Core/FileSystemModule/IEFile.hpp"

void IEShader::destroy() {
	for (const std::function<void()> &function: deletionQueue) {
		function();
	}
	deletionQueue.clear();
}

void IEShader::create(IERenderEngine *renderEngineLink, IEFile *shaderFile) {
	file = shaderFile;
	linkedRenderEngine = renderEngineLink;
	std::vector<std::string> extensions = file->extensions();
	std::string fileContents;
	if (std::find(extensions.begin(), extensions.end(), "spv") == extensions.end()) {
		compile(file->path, file->path + ".spv");
		file = new IEFile{file->path + ".spv"};
		file->open();
		fileContents = file->read(file->length, 0);
		file->close();
	} else {
		file->open();
		fileContents = file->read(file->length, 0);
		file->close();
	}
	VkShaderModuleCreateInfo shaderModuleCreateInfo{
			.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize=static_cast<size_t>(file->length),
			.pCode=reinterpret_cast<const uint32_t *>(fileContents.data()),
	};
	VkResult result = vkCreateShaderModule(linkedRenderEngine->device.device, &shaderModuleCreateInfo, nullptr, &module);
	if (result != VK_SUCCESS) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR,
												 "Failed to create shader module! Error: " + IERenderEngine::translateVkResultCodes(result));
	} else {
		deletionQueue.emplace_back([&] {
			vkDestroyShaderModule(linkedRenderEngine->device.device, module, nullptr);
		});
	}
}

IEShader::~IEShader() {
	destroy();
}

std::function<void(IEShader &, const std::string &, std::string)> IEShader::_compile{nullptr};

void IEShader::compile(const std::string &input, std::string output) {
	_compile(*this, input, output);
}

void IEShader::_vulkanCompile(const std::string &input, std::string output) {
	std::ifstream rawFile(input, std::ios::ate | std::ios::binary);
	if (!rawFile.is_open()) {
		throw std::runtime_error("failed to open shader file: " + input);
	}
	rawFile.close();
	if (output.empty()) {
		output = input + ".spv";
	}
	if (system((GLSLC + input + " -o " + output).c_str()) != 0) {
		throw std::runtime_error("failed to compile shaders: " + input);
	}
}

void IEShader::_openglCompile(const std::string &input, std::string output) {
	// This should be replaced by the filesystem
	std::ifstream rawFile(input, std::ios::ate | std::ios::binary);
	if (!rawFile.is_open()) {
		throw std::runtime_error("failed to open shader file: " + input);
	}
	std::stringstream sstr;
	sstr << rawFile.rdbuf();
	std::string source = sstr.str();
	
	// Compile shader
	GLint result = GL_FALSE;
	int infoLogLength;
	char const *sourcePointer = source.c_str();
	glShaderSource(shaderID, 1, &sourcePointer, nullptr);
	glCompileShader(shaderID);
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
}
