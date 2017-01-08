#version 330

uniform vec3 ambientColor;

out vec3 fragColor;

void main(void)
{
	fragColor = ambientColor;
}
