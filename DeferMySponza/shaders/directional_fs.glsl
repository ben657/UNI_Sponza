#version 330

uniform sampler2DRect positionSampler;
uniform sampler2DRect normalSampler;
uniform sampler2DRect materialSampler;

uniform vec3 cameraPosition;
uniform vec3 ambientIntensity;

out vec3 fragColor;

vec3 phong(vec3 normal, vec3 viewDirection, vec3 lightDirection)
{
	float diffuseIntensity = max(dot(lightDirection, normal), 0.0f);
	float specularIntensity = 0.0f;
	if (diffuseIntensity > 0.0f)
	{
		vec3 halfDirection = normalize(lightDirection + viewDirection);
		float specularAngle = max(dot(halfDirection, normal), 0.0f);
		specularIntensity = pow(specularAngle, material.shininess);
	}

	return material.diffuse * diff_i + material.specular * spec_i;
}

vec3 directionalLight(vec3 normal, vec3 viewDirection)
{
	return light.intensity * phong(normal, viewDirection, light.direction * -1.0f);
}

void main(void)
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 normal = normalize(texelFetch(normalSampler, coord).rgb);
	vec3 position = texelFetch(positionSampler, coord).rgb;

	vec3 viewDirection = normalize(cameraPosition - position);

	fragColor = vec4(directionalLight(normal, viewDirection).rgb, 1.0f);
}
