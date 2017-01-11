#version 330

uniform mat4 projViewMatrix;

in vec3 vertexPosition;
in vec3 vertexNormal;
in mat4 instanceMatrix;
in vec4 instanceMaterial;

out vec3 varyingPosition;
out vec3 varyingNormal;
out vec4 material;

void main()
{
	varyingPosition = vec3(instanceMatrix * vec4(vertexPosition, 1.0));
	varyingNormal = vec3(instanceMatrix * vec4(vertexNormal, 0.0));

	material = instanceMaterial;

	mat4 combinedMatrix = projViewMatrix * instanceMatrix;
	gl_Position = combinedMatrix * vec4(vertexPosition, 1.0);
}