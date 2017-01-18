#version 330

uniform mat4 projViewMatrix;

in vec3 vertexPosition;
in mat4 instanceMatrix;

void main()
{
	mat4 combinedMatrix = projViewMatrix * instanceMatrix;
	gl_Position = combinedMatrix * vec4(vertexPosition, 1.0);
}