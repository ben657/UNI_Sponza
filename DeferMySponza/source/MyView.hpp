#pragma once

#include <scene/scene_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include "ShaderProgram.h"

#include <vector>
#include <map>
#include <memory>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
};

struct Mesh
{
	GLuint vao = 0;
	GLuint vertexVbo = 0;
	GLuint elementVbo = 0;
	GLuint elementCount = 0;
};

struct Material
{
	glm::vec3 diffuseColor;
	float shininess = 0.0f;
};

struct DirectionalLight
{
	glm::vec3 direction;
	int pad0 = 0;
	glm::vec3 intensity;
};

struct PointLight
{
	glm::vec3 position;
	float range;
	glm::vec3 intensity;
};

class MyView : public tygra::WindowViewDelegate
{
public:

    MyView();

    ~MyView();

    void setScene(const scene::Context * scene);

private:

	const GLuint vertexPositionLocation = 0;
	const GLuint vertexNormalLocation = 1;

	GLenum defaultDrawBuffers[2] = { GL_FRONT_LEFT, GL_BACK_LEFT };
	GLuint gBuffer = 0;
	GLenum gBufferDrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	GLuint lBuffer = 0;
	GLenum lBufferDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	GLuint depthStencilBuffer = 0;

	GLuint positionsTexture = 0;
	GLuint normalsTexture = 0;
	GLuint materialsTexture = 0;
	GLuint colorTexture = 0;

	Mesh quadMesh;
	Mesh sphereMesh;
	std::map<scene::MeshId, Mesh> meshes;

	std::map<scene::MaterialId, Material> materials;
	GLuint materialUbo = 0;
	GLuint materialUboIndex = 0;

	GLuint directionalLightUbo = 0;
	GLuint directionalLightUboIndex = 1;

	ShaderProgram* geometryProgram;
	ShaderProgram* ambientProgram;
	ShaderProgram* directionalLightProgram;

	GLuint loadShader(std::string path, GLuint type);
	void buildGeometryPassProgram();
	void buildAmbientPassProgram();
	void buildDirectionalPassProgram();
	void buildPostPassProgram();

	void loadMesh(scene::Mesh mesh);

	void generateQuadMesh();
	void generateSphereMesh();

    void windowViewWillStart(tygra::Window * window) override;

    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;

    void windowViewRender(tygra::Window * window) override;

    const scene::Context * scene_;

};
