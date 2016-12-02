#include "MyView.hpp"

#include <scene/scene.hpp>
#include <tygra/FileHelper.hpp>
#include <tsl/shapes.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::setScene(const scene::Context * scene)
{
    scene_ = scene;
}

GLuint MyView::loadShader(std::string path, GLuint type)
{
	GLuint shader = glCreateShader(type);
	const char* shader_string = tygra::createStringFromFile(path).c_str();
	glShaderSource(shader, 1, (const GLchar**)&shader_string, NULL);
	glCompileShader(shader);

	GLint compileStatus = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus != GL_TRUE)
	{
		GLchar log[1024] = "";
		glGetShaderInfoLog(shader, 1024, NULL, log);
		std::cerr << log << std::endl;
	}

	return shader;
}

void MyView::loadMesh(scene::Mesh mesh)
{
}

void MyView::generateLightMeshes()
{

}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
}

void MyView::windowViewRender(tygra::Window * window)
{
    assert(scene_ != nullptr);

    glClearColor(0.f, 0.f, 0.25f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT);
}
