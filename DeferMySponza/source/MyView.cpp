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

GLuint MyView::buildGeometryPassProgram()
{
	GLuint vertexShader = loadShader("resource:///geom_vs.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("resource:///geom_fs.glsl", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glBindAttribLocation(program, 0, "vertexPosition");
	glBindAttribLocation(program, 0, "vertexNormal");
	glDeleteShader(vertexShader);

	glAttachShader(program, fragmentShader);
	glBindFragDataLocation(program, 0, "gBufferPosition");
	glBindFragDataLocation(program, 1, "gBufferNormal");
	glDeleteShader(fragmentShader);

	glLinkProgram(program);

	GLint linkStatus = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE)
	{
		GLchar log[1024] = "";
		glGetProgramInfoLog(program, 1024, NULL, log);
		std::cerr << log << std::endl;
	}

	return program;
}

GLuint MyView::buildAmbientPassProgram()
{
	return 0;
}

void MyView::loadMesh(scene::Mesh mesh)
{
}

void MyView::generateQuadMesh()
{
	Mesh mesh;
	GLuint vertexVbo = 0;
	GLuint elementVbo = 0;

	std::vector<glm::vec2> vertices;
	vertices[0] = glm::vec2(-1, -1);
	vertices[1] = glm::vec2(1, -1);
	vertices[2] = glm::vec2(1, 1);
	vertices[3] = glm::vec2(-1, 1);

	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);

	glGenBuffers(1, &vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void MyView::generateSphereMesh()
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
