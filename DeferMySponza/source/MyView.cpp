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

GLuint MyView::buildDirectionalPassProgram()
{
	GLuint vertexShader = loadShader("resource:///directional_vs.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = loadShader("resource:///directional_fs.glsl", GL_FRAGMENT_SHADER);

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
	glGenFramebuffers(1, &lBuffer);

	glGenRenderbuffers(1, &depthStencilBuffer);
	glGenTextures(1, &positionsTexture);
	glGenTextures(1, &normalsTexture);
	glGenTextures(1, &colorTexture);
	
	generateQuadMesh();

	geometryPassProgram = buildGeometryPassProgram();
	directionalPassProgram = buildDirectionalPassProgram();

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
	
	//Create depth-stencil buffer to share between gBuffer and lBuffer
	glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, lBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//Create position and normal textures to be written to from the gBuffer and used by shaders
	glBindTexture(GL_TEXTURE_RECTANGLE, positionsTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, positionsTexture, 0);
	
	glBindTexture(GL_TEXTURE_RECTANGLE, normalsTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, normalsTexture, 0);

	//Create texture to hold results of shading in lBuffer
	glBindTexture(GL_TEXTURE_RECTANGLE, colorTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, lBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, colorTexture, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	GLint framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "gBuffer not complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, lBuffer);
	GLint framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "lBuffer not complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	glDeleteFramebuffers(1, &gBuffer);
	glDeleteFramebuffers(1, &lBuffer);

	glDeleteRenderbuffers(1, &depthStencilBuffer);
	glDeleteTextures(1, &positionsTexture);
	glDeleteTextures(1, &normalsTexture);
	glDeleteTextures(1, &colorTexture);
}

void MyView::windowViewRender(tygra::Window * window)
{
    assert(scene_ != nullptr);
	
	//Geometry pass settings
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, ~0);
	glStencilMask(~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glDisable(GL_BLEND);

	glCullFace(GL_BACK);

	//Set draw buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glDrawBuffers(2, gBufferDrawBuffers);

	//SCM-Purple ish?
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//Set up shading variables
	glUseProgram(geometryPassProgram);

	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	int width = viewportSize[2];
	int height = viewportSize[3];
	const float aspectRatio = (float)width / (float)height;
	glm::mat4 projectionMatrix = glm::perspective(90.f, aspectRatio, 1.f, 1000.f);

	const scene::Camera& camera = scene_->getCamera();
	glm::vec3 cameraPos = (const glm::vec3&)camera.getPosition();
	glm::vec3 cameraDir = (const glm::vec3&)camera.getDirection();
	glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0.f, 1.f, 0.f));
	
	glm::mat4 projViewMatrix = projectionMatrix * viewMatrix;

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

	//Bind lBuffer, do light shading
	glDisable(GL_DEPTH_TEST);

	//Don't shade fragments not part of Sponza
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glBindFramebuffer(GL_FRAMEBUFFER, lBuffer);
	glDrawBuffers(1, lBufferDrawBuffers);

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(directionalPassProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, positionsTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_RECTANGLE, normalsTexture);

	GLuint posSamplerID = glGetUniformLocation(directionalPassProgram, "positionSampler");
	GLuint normalSamplerID = glGetUniformLocation(directionalPassProgram, "normalSampler");
	glUniform1i(posSamplerID, 0);
	glUniform1i(normalSamplerID, 1);

	glBindVertexArray(quadMesh.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
