#pragma once

#include <tgl/tgl.h>

template <class T>
class UniformBuffer
{
private:
	GLuint ubo = 0;
	GLuint index = 0;

public:
	UniformBuffer(GLuint index);
	~UniformBuffer();
};

