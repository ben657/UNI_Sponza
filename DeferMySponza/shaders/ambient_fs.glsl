#version 330

uniform sampler2DRect positionSampler;
uniform sampler2DRect normalSampler;

uniform vec3 ambientIntensity;

out vec3 fragColor;

void main(void)
{
	//Output normal as colour just to see it's working
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 normal = texelFetch(normalSampler, coord).rgb;

	fragColor = (normal + vec3(1.0, 1.0, 1.0)) * 0.5f;
	//fragColor = ambientIntensity;
}
