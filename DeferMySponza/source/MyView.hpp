#pragma once

#include <scene/scene_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

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

class MyView : public tygra::WindowViewDelegate
{
public:

    MyView();

    ~MyView();

    void setScene(const scene::Context * scene);

private:

	const GLuint vertexPositionLocation = 0;
	const GLuint vertexNormalLocation = 1;

	GLuint gBuffer = 0;
	GLuint gBufferDepthStencil = 0;
	GLuint gBufferPositions = 0;
	GLuint gBufferNormals = 0;

	Mesh quadMesh;
	std::map<scene::MeshId, Mesh> meshes;

	GLuint geometryPassProgram = 0;
	GLuint ambientPassProgram = 0;
	GLuint lightingPassProgram = 0;

	GLuint loadShader(std::string path, GLuint type);
	GLuint buildGeometryPassProgram();
	GLuint buildAmbientPassProgram();

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
