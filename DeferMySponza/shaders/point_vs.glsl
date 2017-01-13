#version 330

uniform mat4 projViewMatrix;

in vec3 vertexPosition;
in mat4 instanceMatrix;
in vec3 lightPosition;
in vec3 lightIntensity;
in float lightRange;

out Light
{
	vec3 position;
	vec3 intensity;
	float range;
} light;

void main(void)
{
	light.position = lightPosition;
	light.intensity = lightIntensity;
	light.range = lightRange;

	mat4 combinedMatrix = projViewMatrix * instanceMatrix;
	gl_Position = combinedMatrix * vec4(vertexPosition, 1.0);
}