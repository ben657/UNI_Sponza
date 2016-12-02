#version 330

uniform mat4 projViewMatrix;
uniform mat4 modelMatrix;

in vec3 vertexPositon;
in vec3 vertexNormal;

out vec3 varyingPosition;
out vec3 varyingNormal;

void main()
{
	varyingPosition = vec3(modelMatrix * vec4(vertexPositon, 1.0));
	varyingNormal = vec3(modelMatrix * vec4(vertexNormal, 0.0));

	mat4 combinedMatrix = projViewMatrix * modelMatrix;
	gl_Position = combinedMatrix * vec4(vertexPositon, 1.0);
}