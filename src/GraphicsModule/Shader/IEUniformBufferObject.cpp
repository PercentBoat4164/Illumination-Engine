/* Include this file's header. */
#include "IEUniformBufferObject.hpp"

void IEUniformBufferObject::openglUploadUniform(GLint program) {
	glUseProgram(program);
	glUniformMatrix4fv(glGetUniformLocation(program, "projectionViewModelMatrix"), 1, GL_FALSE, &projectionViewModelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);
	glUniform3fv(glGetUniformLocation(program, "position"), 1, &position[0]);
	glUniform1f(glGetUniformLocation(program, "time"), time);
	glUseProgram(0);
}
