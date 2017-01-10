#pragma once

#include <tgl/tgl.h>

template <class T>
class UniformBuffer
{
private:
	GLuint ubo = 0;
	GLuint index = 0;

public:
	UniformBuffer(GLuint index, GLenum usage, GLuint count = 1);
	~UniformBuffer();

	void bufferData(const T* data, GLuint index = 0);
	GLuint getLocation() { return ubo; }
	GLuint getIndex() { return index; }
};

template<class T>
inline UniformBuffer<T>::UniformBuffer(GLuint index, GLenum usage, GLuint count) : index(index)
{
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * count, 0, usage);
	glBindBufferBase(GL_UNIFORM_BUFFER, index, ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template<class T>
inline UniformBuffer<T>::~UniformBuffer()
{
}

template<class T>
inline void UniformBuffer<T>::bufferData(const T * data, GLuint index)
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(T) * index, sizeof(T), data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
