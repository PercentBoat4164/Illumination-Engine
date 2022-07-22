/* Include this file's header. */
#include "IEUniformBufferObject.hpp"

void IEUniformBufferObject::openglUploadUniform(const std::string &name, GLint program) {
	glUniformMatrix4fv(glGetUniformLocation(program, (name + ".viewModelMatrix").c_str()), 1, GL_FALSE, &viewModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, (name + ".modelMatrix").c_str()), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, (name + ".projectionMatrix").c_str()), 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, (name + ".normalMatrix").c_str()), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(program, (name + ".position").c_str()), 1, &position[0]);
	glUniform1f(glGetUniformLocation(program, (name + ".time").c_str()), time);
}
