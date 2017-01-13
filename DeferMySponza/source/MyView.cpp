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

void MyView::buildGeometryPassProgram()
{
	//TODO: set inputs/outputs manually, REMEMBER MATRIX STUFF, change draws to instanced
	geometryProgram = new ShaderProgram("resource:///geom_vs.glsl", "resource:///geom_fs.glsl");
	geometryProgram->setVertexInput(0, "vertexPosition");
	geometryProgram->setVertexInput(1, "vertexNormal");
	geometryProgram->setVertexInput(2, "instanceMatrix");
	geometryProgram->setVertexInput(6, "materialDiffuse");
	geometryProgram->setVertexInput(7, "materialShininess");
	geometryProgram->setFragmentOutput(0, "gBufferPosition");
	geometryProgram->setFragmentOutput(1, "gBufferNormal");
	geometryProgram->setFragmentOutput(2, "gBufferMaterial");
	geometryProgram->build();
}

void MyView::buildShadowProgram()
{
	shadowProgram = new ShaderProgram("resource:///shadow_vs.glsl", "resource:///shadow_fs.glsl");
	shadowProgram->setVertexInput(0, "vertexPosition");
	shadowProgram->setFragmentOutput(0, "fragDepth");
	GLuint status = shadowProgram->build();
}

void MyView::buildAmbientPassProgram()
{
	ambientProgram = new ShaderProgram("resource:///ambient_vs.glsl", "resource:///ambient_fs.glsl");
	ambientProgram->setVertexInput(0, "vertexPosition");
	ambientProgram->setFragmentOutput(0, "fragColor");
	GLuint status = ambientProgram->build();
	
	const glm::vec3& ambientColor = (const glm::vec3&)scene_->getAmbientLightIntensity();
	ambientProgram->uploadVector3Uniform(ambientColor, "ambientColor");
}

void MyView::buildDirectionalPassProgram()
{
	directionalLightProgram = new ShaderProgram("resource:///directional_vs.glsl", "resource:///directional_fs.glsl");
	directionalLightProgram->setVertexInput(0, "vertexPosition");
	directionalLightProgram->setFragmentOutput(0, "fragColor");
	GLuint status = directionalLightProgram->build();

	directionalLightProgram->bindUniformBuffer("DirectionalLight", directionalLightUbo->getIndex());
}

void MyView::buildPointPassProgram()
{
	pointLightProgram = new ShaderProgram("resource:///point_vs.glsl", "resource:///point_fs.glsl");
	pointLightProgram->setVertexInput(0, "vertexPosition");
	pointLightProgram->setVertexInput(1, "instanceMatrix");
	pointLightProgram->setVertexInput(5, "lightPosition");
	pointLightProgram->setVertexInput(6, "lightIntensity");
	pointLightProgram->setVertexInput(7, "lightRange");
	pointLightProgram->setFragmentOutput(0, "fragColor");
	GLuint status = pointLightProgram->build();
}

void MyView::buildPostPassProgram()
{
	
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

	const std::vector<scene::InstanceId>& instanceIds = scene_->getInstancesByMeshId(mesh.getId());
	glGenBuffers(1, &newMesh.instanceVbo);
	glBindBuffer(GL_ARRAY_BUFFER, newMesh.instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, instanceIds.size() * sizeof(MeshInstance), 0, GL_DYNAMIC_DRAW);
	updateMeshInstances(newMesh, mesh.getId());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &newMesh.elementVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.elementVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &newMesh.vao);
	glBindVertexArray(newMesh.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.elementVbo);
	glBindBuffer(GL_ARRAY_BUFFER, newMesh.vertexVbo);
	int attrib = 0;
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib++, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(0));
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib++, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), TGL_BUFFER_OFFSET(sizeof(glm::vec3)));

	glBindBuffer(GL_ARRAY_BUFFER, newMesh.instanceVbo);
	for (int i = 0; i < 4; i++)
	{
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 4, GL_FLOAT, GL_FALSE, sizeof(MeshInstance), TGL_BUFFER_OFFSET(sizeof(glm::vec4) * i));
		glVertexAttribDivisor(attrib++, 1);
	}
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 4, GL_FLOAT, GL_FALSE, sizeof(MeshInstance), TGL_BUFFER_OFFSET(sizeof(glm::mat4)));
	glVertexAttribDivisor(attrib++, 1);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	meshes[mesh.getId()] = newMesh;
}

void MyView::updateMeshInstances(Mesh& mesh, const scene::MeshId& meshId)
{
	std::vector<MeshInstance> instances;
	const std::vector<scene::InstanceId>& instanceIds = scene_->getInstancesByMeshId(meshId);
	for (const scene::InstanceId& id : instanceIds)
	{
		const scene::Instance& instance = scene_->getInstanceById(id);
		MeshInstance newInstance;

		scene::Matrix4x3 sceneMatrix = instance.getTransformationMatrix();
		glm::mat4x3 matrix = (const glm::mat4x3&)sceneMatrix;
		newInstance.transformationMatrix = matrix;

		scene::Material material = scene_->getMaterialById(instance.getMaterialId());
		newInstance.material = (const glm::vec4&)material.getDiffuseColour();
		newInstance.material.a = material.getShininess();

		instances.push_back(newInstance);
	}
	mesh.instanceCount = instances.size();

	glBindBuffer(GL_ARRAY_BUFFER, mesh.instanceVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(MeshInstance) * instances.size(), instances.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	tsl::IndexedMeshPtr mesh = tsl::createSpherePtr(1.f, 12);
	mesh = tsl::cloneIndexedMeshAsTriangleListPtr(mesh.get());

	sphereMesh.elementCount = mesh->indexCount();

	glGenBuffers(1, &sphereMesh.vertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, sphereMesh.vertexVbo);
	glBufferData(GL_ARRAY_BUFFER,
		mesh->vertexCount() * sizeof(glm::vec3),
		mesh->positionArray(),
		GL_STATIC_DRAW);

	const std::vector<scene::PointLight>& lights = scene_->getAllPointLights();
	glGenBuffers(1, &sphereMesh.instanceVbo);
	glBindBuffer(GL_ARRAY_BUFFER, sphereMesh.instanceVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointLightInstance) * lights.size(), 0, GL_DYNAMIC_DRAW);
	updatePointLightInstances();
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &sphereMesh.elementVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereMesh.elementVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		mesh->indexCount() * sizeof(unsigned int),
		mesh->indexArray(),
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &sphereMesh.vao);
	glBindVertexArray(sphereMesh.vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereMesh.elementVbo);
	glBindBuffer(GL_ARRAY_BUFFER, sphereMesh.vertexVbo);
	int attrib = 0;
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib++, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, sphereMesh.instanceVbo);
	for (int i = 0; i < 4; i++)
	{
		glEnableVertexAttribArray(attrib);
		glVertexAttribPointer(attrib, 4, GL_FLOAT, GL_FALSE, sizeof(PointLightInstance), TGL_BUFFER_OFFSET(sizeof(glm::vec4) * i));
		glVertexAttribDivisor(attrib++, 1);
	}
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInstance), TGL_BUFFER_OFFSET(sizeof(glm::mat4)));
	glVertexAttribDivisor(attrib++, 1);

	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, sizeof(PointLightInstance), TGL_BUFFER_OFFSET(sizeof(glm::mat4) + sizeof(glm::vec3)));
	glVertexAttribDivisor(attrib++, 1);

	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 1, GL_FLOAT, GL_FALSE, sizeof(PointLightInstance), TGL_BUFFER_OFFSET(sizeof(glm::mat4) + sizeof(glm::vec3) * 2));
	glVertexAttribDivisor(attrib++, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void MyView::updatePointLightInstances()
{
	std::vector<PointLightInstance> instances;
	const std::vector<scene::PointLight>& lights = scene_->getAllPointLights();
	for (const scene::PointLight& light : lights)
	{
		PointLightInstance newInstance;
		glm::mat4 modelMatrix = glm::mat4();
		modelMatrix = glm::translate(modelMatrix, (const glm::vec3&)light.getPosition());
		modelMatrix = glm::scale(modelMatrix, glm::vec3(light.getRange()));
		newInstance.transformationMatrix = modelMatrix;

		newInstance.position = (const glm::vec3&)light.getPosition();
		newInstance.range = light.getRange();
		newInstance.intensity = (const glm::vec3&)light.getIntensity();
		instances.push_back(newInstance);
	}
	sphereMesh.instanceCount = instances.size();

	glBindBuffer(GL_ARRAY_BUFFER, sphereMesh.instanceVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PointLightInstance) * instances.size(), instances.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);
	
	glGenFramebuffers(1, &gBuffer);
	glGenFramebuffers(1, &lBuffer);

	glGenRenderbuffers(1, &depthStencilBuffer);
	glGenTextures(1, &positionsTexture);
	glGenTextures(1, &normalsTexture);
	glGenTextures(1, &materialsTexture);
	glGenTextures(1, &colorTexture);

	glGenFramebuffers(1, &shadowBuffer);
	glGenTextures(1, &shadowMap);

	directionalLightUbo = new UniformBuffer<DirectionalLight>(1, GL_STREAM_DRAW);

	buildGeometryPassProgram();
	buildAmbientPassProgram();
	buildDirectionalPassProgram();
	buildPointPassProgram();

	generateQuadMesh();
	generateSphereMesh();

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
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	glBindTexture(GL_TEXTURE_RECTANGLE, positionsTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, positionsTexture, 0);
	
	glBindTexture(GL_TEXTURE_RECTANGLE, normalsTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, normalsTexture, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, materialsTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, materialsTexture, 0);

	GLint framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "gBuffer not complete");
	}

	//Create texture to hold results of shading in lBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, lBuffer);

	glBindTexture(GL_TEXTURE_RECTANGLE, colorTexture);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, colorTexture, 0);

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "lBuffer not complete");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);

	glBindTexture(GL_TEXTURE_RECTANGLE, shadowMap);
	glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_DEPTH_COMPONENT16, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_RECTANGLE, shadowMap, 0);

	framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
		tglDebugMessage(GL_DEBUG_SEVERITY_HIGH_ARB, "shadowBuffer not complete");
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

void MyView::enableGeometrySettings()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, ~0);
	glStencilMask(~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glDisable(GL_BLEND);

	glCullFace(GL_BACK);
}

void MyView::enableAmbientSettings()
{
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glDisable(GL_BLEND);

	glCullFace(GL_BACK);
}

void MyView::enableShadowSettings()
{
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

void MyView::enableDirectionalLightSettings()
{
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glCullFace(GL_BACK);
}

void MyView::enablePointLightSettings()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_GREATER);
	
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 1, ~0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glCullFace(GL_FRONT);
}

void MyView::drawSponza()
{
	for (auto& iterator = meshes.begin(); iterator != meshes.end(); iterator++)
	{
		Mesh mesh = iterator->second;
		updateMeshInstances(mesh, iterator->first);

		glBindVertexArray(mesh.vao);
		glDrawElementsInstanced(GL_TRIANGLES, mesh.elementCount, GL_UNSIGNED_INT, 0, mesh.instanceCount);
	}
}

void MyView::drawQuad()
{
	glBindVertexArray(quadMesh.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void MyView::drawSpheres()
{
	glBindVertexArray(sphereMesh.vao);
	glDrawElementsInstanced(GL_TRIANGLES, sphereMesh.elementCount, GL_UNSIGNED_INT, 0, sphereMesh.instanceCount);
}

void MyView::windowViewRender(tygra::Window * window)
{
    assert(scene_ != nullptr);
	
	enableGeometrySettings();

	//Set draw buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glDrawBuffers(3, gBufferDrawBuffers);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//Calculate shared shading variables
	GLint viewportSize[4];
	glGetIntegerv(GL_VIEWPORT, viewportSize);
	int width = viewportSize[2];
	int height = viewportSize[3];
	const float aspectRatio = (float)width / (float)height;
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), aspectRatio, 1.f, 1000.f);

	const scene::Camera& camera = scene_->getCamera();
	glm::vec3 cameraPos = (const glm::vec3&)camera.getPosition();
	glm::vec3 cameraDir = (const glm::vec3&)camera.getDirection();
	glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraDir, glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 projViewMatrix = projectionMatrix * viewMatrix;

	//Render geometry data into gBuffer
	geometryProgram->uploadMatrixUniform(projViewMatrix, "projViewMatrix");
	glUseProgram(geometryProgram->getProgram());
	drawSponza();
	
	//Set up for drawing to lBuffer
	enableAmbientSettings();

	//Set lBuffer as active framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, lBuffer);
	glDrawBuffers(1, lBufferDrawBuffers);

	glClearColor(0.33f, 0.22f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//Render only ambient light where stencil test passes
	glUseProgram(ambientProgram->getProgram());
	drawQuad();
	
	//Additive blend on top of ambient light
	enableDirectionalLightSettings();

	directionalLightProgram->activateTextureSamplerUniform(0, positionsTexture, "positionSampler");
	directionalLightProgram->activateTextureSamplerUniform(1, normalsTexture, "normalSampler");
	directionalLightProgram->activateTextureSamplerUniform(2, materialsTexture, "materialSampler");
	directionalLightProgram->uploadVector3Uniform(cameraPos, "cameraPosition");
	glUseProgram(directionalLightProgram->getProgram());

	glm::mat4 shadowProjectionMatrix = glm::ortho(-10, 10, -10, 10, -10, 20);

	const std::vector<scene::DirectionalLight>& directionalLights = scene_->getAllDirectionalLights();
	DirectionalLight dLight;
	for (const scene::DirectionalLight& light : directionalLights)
	{
		const glm::vec3& direction = (const glm::vec3&)light.getDirection();
		/*glm::mat4 shadowViewMatrix = glm::lookAt(direction * -1.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
		glDrawBuffers(1, shadowDrawBuffers);*/

		/*enableShadowSettings();
		shadowProgram->uploadMatrixUniform(shadowProjectionMatrix * shadowViewMatrix, "combinedMatrix");

		drawSponza();*/

		//enableDirectionalLightSettings();

		dLight.direction = direction;
		dLight.intensity = (const glm::vec3&) light.getIntensity();
		directionalLightUbo->bufferData(&dLight);
		drawQuad();
	}

	//Re-enable depth test for point lights
	enablePointLightSettings();
	
	pointLightProgram->activateTextureSamplerUniform(0, positionsTexture, "positionSampler");
	pointLightProgram->activateTextureSamplerUniform(1, normalsTexture, "normalSampler");
	pointLightProgram->activateTextureSamplerUniform(2, materialsTexture, "materialSampler");
	pointLightProgram->uploadMatrixUniform(projViewMatrix, "projViewMatrix");
	pointLightProgram->uploadVector3Uniform(cameraPos, "cameraPosition");
	glUseProgram(pointLightProgram->getProgram());

	updatePointLightInstances();
	drawSpheres();

	//Blit lBuffer to screen
	glBindFramebuffer(GL_READ_FRAMEBUFFER, lBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
