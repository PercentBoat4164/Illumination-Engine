#pragma once

/* Define macros used throughout the file. */
#ifdef WIN32
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <vector>
#include <functional>
#include "Core/FileSystemModule/IEFile.hpp"
#include <GL/glew.h>


class IEShader {
public:
	std::vector<char> data;
	std::vector<std::function<void()>> deletionQueue;
	VkShaderModule module;
	IERenderEngine *linkedRenderEngine;
	IEFile *file;
	GLuint shaderID;
	
private:
	static std::function<void(IEShader &, IERenderEngine *, IEFile *)> _create;
	static std::function<void(IEShader &, const std::string &, std::string)> _compile;
	
protected:
	void _openglCompile(const std::string &, std::string);
	
	void _vulkanCompile(const std::string &, std::string);
	
public:
	void destroy();

	~IEShader();

	void compile(const std::string& input, std::string output = "");

	void create(IERenderEngine *renderEngineLink, IEFile *shaderFile);
};