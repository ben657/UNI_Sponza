#version 330

uniform sampler2DRect screenSampler;

out vec3 fragColor;

void main(void)
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 color = texelFetch(screenSampler, coord).xyz;

	fragColor = color;
}
