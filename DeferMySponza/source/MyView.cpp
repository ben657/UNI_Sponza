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
	std::string shaderString = tygra::createStringFromFile(path);
	const char* shaderSource = shaderString.c_str();
	glShaderSource(shader, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(shader);

	return shader;
}

GLuint MyView::buildGeometryPassProgram()
{
	GLuint vertexShader = loadShader("resource:///geom_vs.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("resource:///geom_fs.glsl", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glBindAttribLocation(program, 0, "vertexPosition");
	glBindAttribLocation(program, 1, "vertexNormal");
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
	GLuint vertexShader = loadShader("resource:///ambient_vs.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("resource:///ambient_fs.glsl", GL_FRAGMENT_SHADER);

	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glBindAttribLocation(program, 0, "vertexPosition");
	glDeleteShader(vertexShader);

	glAttachShader(program, fragmentShader);
	glBindFragDataLocation(program, 0, "fragColour");
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

void MyView::loadMesh(scene::Mesh mesh)
{
	Mesh newMesh;

	const std::vector<scene::Vector3> positions = mesh.getPositionArray();
	const std::vector<scene::Vector3> normals = mesh.getNormalArray();
	std::vector<Vertex> vertexes;
	//Add values into an array of vertices for the vertex buffer
	for (int i = 0; i < positions.size(); i++)
	{
		Vertex v;
		v.position = (const glm::vec3&)positions[i];
		v.normal = (const glm::vec3&)normals[i];
		vertexes.push_back(v);
	}

	const std::vector<unsigned int> elements = mesh.getElementArray();
	newMesh.elementCount = elements.size();

	//One interleaved vertex buffer
	glGenBuffers(1, &newMesh.vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, newMesh.vertexVbo);
	glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(Vertex), vertexes.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &newMesh.elementVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.elementVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &newMesh.vao);
	glBindVertexArray(newMesh.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.elementVbo);
	glBindBuffer(GL_ARRAY_BUFFER, newMesh.vertexVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(sizeof(glm::vec3)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	meshes[mesh.getId()] = newMesh;
}

void MyView::generateQuadMesh()
{
	GLuint vertexVbo = 0;
	GLuint elementVbo = 0;

	std::vector<glm::vec2> vertices(4);
	vertices[0] = glm::vec2(-1, -1);
	vertices[1] = glm::vec2(1, -1);
	vertices[2] = glm::vec2(1, 1);
	vertices[3] = glm::vec2(-1, 1);

	glGenVertexArrays(1, &quadMesh.vao);
	glBindVertexArray(quadMesh.vao);

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
	
	glGenFramebuffers(1, &gBuffer);
	//TODO: generate gbuffer, render to it
	glGenRenderbuffers(1, &gBufferDepthStencil);
	glGenTextures(1, &gBufferPositions);
	glGenTextures(1, &gBufferNormals);
	
	generateQuadMesh();

	geometryPassProgram = buildGeometryPassProgram();
	ambientPassProgram = buildAmbientPassProgram();

	scene::GeometryBuilder builder;
	for (const scene::Mesh& mesh : builder.getAllMeshes())
	{
		loadMesh(mesh);
	}
}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	
	glBindRenderbuffer(GL_RENDERBUFFER, gBufferDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gBufferDepthStencil);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, gBufferPositions);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, gBufferPositions, 0);
	
	glBindTexture(GL_TEXTURE_RECTANGLE, gBufferNormals);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, gBufferNormals, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	GLint framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "framebuffer not complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
}

void MyView::windowViewRender(tygra::Window * window)
{
    assert(scene_ != nullptr);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLenum gBufferBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, gBufferBuffers);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, ~0);
	glStencilMask(~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glDisable(GL_BLEND);

	//SCM-Purple ish?
    glClearColor(0.4f, 0.15f, 0.6f, 0.f);
	glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glCullFace(GL_BACK);

	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	const float aspectRatio = (float)viewportSize[2] / (float)viewportSize[3];
	glm::mat4 projectionMatrix = glm::perspective(90.f, aspectRatio, 1.f, 1000.f);

	const scene::Camera& camera = scene_->getCamera();
	glm::vec3 cameraPos = (const glm::vec3&)camera.getPosition();
	glm::vec3 cameraDir = (const glm::vec3&)camera.getDirection();
	glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0.f, 1.f, 0.f));
	
	glm::mat4 projViewMatrix = projectionMatrix * viewMatrix;

	glUseProgram(geometryPassProgram);

	GLuint projViewMatrixID = glGetUniformLocation(geometryPassProgram, "projViewMatrix");
	GLuint modelMatrixID = glGetUniformLocation(geometryPassProgram, "modelMatrix");

	glUniformMatrix4fv(projViewMatrixID, 1, GL_FALSE, glm::value_ptr(projViewMatrix));

	for (auto& iterator = meshes.begin(); iterator != meshes.end(); iterator++)
	{
		Mesh mesh = iterator->second;
		std::vector<scene::InstanceId> instances = scene_->getInstancesByMeshId(iterator->first);
		for (scene::InstanceId id : instances)
		{
			scene::Instance instance = scene_->getInstanceById(id);

			glm::mat4 modelMatrix = (glm::mat4)((const glm::mat4x3&)instance.getTransformationMatrix());
			glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			glBindVertexArray(mesh.vao);
			glDrawElements(GL_TRIANGLES, mesh.elementCount, GL_UNSIGNED_INT, 0);
		}
	}

	//Doing this next section just to make sure the gbuffer data is correct outside of nsight
	//Will be replaced with an lbuffer

	int width = viewportSize[2];
	int height = viewportSize[3];

	//read framebuffer still set to gbuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	GLenum defaultBuffers[] = { GL_FRONT_LEFT, GL_BACK_LEFT };
	glDrawBuffers(2, defaultBuffers);

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, gBufferPositions);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, gBufferNormals);

	glUseProgram(ambientPassProgram);

	GLuint posSamplerID = glGetUniformLocation(ambientPassProgram, "positionSampler");
	GLuint normalSamplerID = glGetUniformLocation(ambientPassProgram, "normalSampler");
	glUniform1i(posSamplerID, 0);
	glUniform1i(normalSamplerID, 1);

	glBindVertexArray(quadMesh.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
