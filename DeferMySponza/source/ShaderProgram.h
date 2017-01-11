#pragma once

#include <iostream>
#include <string>
#include <map>

#include <tygra/FileHelper.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram
{
private:
	GLuint program = 0;
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;

	std::map<int, const char*> vertexInputs;
	std::map<int, const char*> fragmentOutputs;

	GLuint loadShader(const std::string& path, GLuint type);

public:
	ShaderProgram(const std::string& vertexFile, const std::string& fragmentFile);
	~ShaderProgram();

	void setVertexInput(int location, const char* inputName) { vertexInputs[location] = inputName; }
	void setFragmentOutput(int location, const char* outputName) { fragmentOutputs[location] = outputName; }
	
	GLint build();
	GLuint& getProgram() { return program; }

	void bindUniformBuffer(const char* uniformName, const GLuint& uniformBufferIndex);
	void uploadMatrixUniform(const glm::mat4& matrix, const char* uniformName);
	void uploadVector3Uniform(const glm::vec3& vector, const char* uniformName);
	void activateTextureSamplerUniform(const GLuint textureUnit, GLuint texture, const char* uniformName);
};
