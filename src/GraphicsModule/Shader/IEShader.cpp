/* Include this file's header. */
#include "IEShader.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"


/* Include dependencies from Core. */
#include "Core/FileSystemModule/IEFile.hpp"

void IEShader::setAPI(const IEAPI &API) {
	if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_create = &IEShader::_openglCreate;
		_compile = &IEShader::_openglCompile;
	} else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_create = &IEShader::_vulkanCreate;
		_compile = &IEShader::_vulkanCompile;
	}
}

void IEShader::destroy() {
	for (const std::function<void()> &function: deletionQueue) {
		function();
	}
	deletionQueue.clear();
}


std::function<void(IEShader &, IERenderEngine *, IEFile *)> IEShader::_create = nullptr;

void IEShader::create(IERenderEngine *engineLink, IEFile *shaderFile) {
	_create(*this, engineLink, shaderFile);
}

void IEShader::_openglCreate(IERenderEngine *renderEngineLink, IEFile *shaderFile) {
	file = shaderFile;
	linkedRenderEngine = renderEngineLink;
	GLenum shaderType = GL_VERTEX_SHADER;
	if (file->extensions()[0] == "frag") shaderType = GL_FRAGMENT_SHADER;
	shaderID = glCreateShader(shaderType);
	compile(file->path, file->path);
}

void IEShader::_vulkanCreate(IERenderEngine *renderEngineLink, IEFile *shaderFile) {
	file = shaderFile;
	linkedRenderEngine = renderEngineLink;
	std::vector<std::string> extensions = file->extensions();
	std::string fileContents;
	if (std::find(extensions.begin(), extensions.end(), "spv") == extensions.end()) {
		compile(file->path, file->path + ".spv");
		file = new IEFile{file->path + ".spv"};
	}
	file->open();
	fileContents = file->read(file->length, 0);
	file->close();
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

void IEShader::compile(const std::string &input, const std::string &output) {
	_compile(*this, input, output);
}

void IEShader::_openglCompile(const std::string &input, std::string) {
	file->open();
	std::string contents = file->read((size_t) file->length, 0);

	// Compile shader
	GLint result = GL_FALSE;
	int infoLogLength;
	const GLchar *shader = contents.c_str();
	glShaderSource(shaderID, 1, &shader, nullptr);
	glCompileShader(shaderID);
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> shaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(shaderID, infoLogLength, nullptr, &shaderErrorMessage[0]);
		printf("%s\n", &shaderErrorMessage[0]);
	}
}

void IEShader::_vulkanCompile(const std::string &input, std::string output) {
	if (output.empty()) {
		output = input + ".spv";
	}
//	if (system((GLSLC + input + " -o " + output).c_str()) != 0) {
//		throw std::runtime_error("failed to compile shaders: " + input);
//	}
}
