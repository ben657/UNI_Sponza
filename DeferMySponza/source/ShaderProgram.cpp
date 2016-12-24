#include "ShaderProgram.h"

GLuint ShaderProgram::loadShader(const std::string & path, GLuint type)
{
	GLuint shader = glCreateShader(type);
	std::string shaderString = tygra::createStringFromFile(path);
	const char* shaderSource = shaderString.c_str();
	glShaderSource(shader, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(shader);

	return shader;
}

ShaderProgram::ShaderProgram(const std::string & vertexFile, const std::string & fragmentFile)
{
	vertexShader = loadShader(vertexFile, GL_VERTEX_SHADER);
	fragmentShader = loadShader(fragmentFile, GL_FRAGMENT_SHADER);
}

ShaderProgram::~ShaderProgram()
{
}

GLint ShaderProgram::build()
{
	program = glCreateProgram();

	glAttachShader(program, vertexShader);
	for (int i = 0; i < vertexInputCount; i++)
	{
		glBindAttribLocation(program, i, vertexInputs[i]);
	}
	
	glAttachShader(program, fragmentShader);
	for (int i = 0; i < fragmentOutputCount; i++)
	{
		glBindFragDataLocation(program, i, fragmentOutputs[i]);
	}

	GLint linkStatus = 0;
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

	return linkStatus;
}

void ShaderProgram::bindUniformBuffer(const char * uniformName, const GLuint & uniformBufferIndex)
{
	GLuint blockIndex = glGetUniformBlockIndex(program, uniformName);
	glUniformBlockBinding(program, blockIndex, uniformBufferIndex);
}

void ShaderProgram::uploadMatrixUniform(const glm::mat4 & matrix, const char * uniformName)
{
	GLuint location = glGetUniformLocation(program, uniformName);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ShaderProgram::uploadVector3Uniform(const glm::vec3 & vector, const char * uniformName)
{
	GLuint location = glGetUniformLocation(program, uniformName);
	glUniform3fv(location, 1, glm::value_ptr(vector));
}

void ShaderProgram::activateTextureSamplerUniform(const GLuint textureUnit, GLuint texture, const char * uniformName)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_RECTANGLE, texture);
	GLuint location = glGetUniformLocation(program, uniformName);
	glUniform1i(location, textureUnit);
}
