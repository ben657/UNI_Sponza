#version 330

struct Material 
{
	vec3 color;
	float shininess;
} material;

layout(std140) uniform SpotLight
{
	vec3 position;
	float range;
	vec3 direction;
	float cutoff;
	vec3 intensity;
} light;

uniform sampler2DRect positionSampler;
uniform sampler2DRect normalSampler;
uniform sampler2DRect materialSampler;

uniform vec3 cameraPosition;

out vec3 fragColor;

vec3 phong(vec3 normal, vec3 viewDirection, vec3 lightDirection)
{
	float diffuseIntensity = max(dot(lightDirection, normal), 0.0f);
	float specularIntensity = 0.0f;
	if (diffuseIntensity > 0.0f && material.shininess > 0.0f)
	{
		vec3 halfDirection = normalize(lightDirection + viewDirection);
		float specularAngle = max(dot(halfDirection, normal), 0.0f);
		specularIntensity = pow(specularAngle, material.shininess);
	}

	return material.color * diffuseIntensity;// +vec3(1.0f, 1.0f, 1.0f) * specularIntensity;
}

vec3 spotLight(vec3 normal, vec3 viewDirection, vec3 lightDirection, float distanceToLight)
{
	float spot = dot(lightDirection, -light.direction);
	if (spot > light.cutoff)
	{
		float attenuation = smoothstep(light.range, 0, distanceToLight);
		vec3 l = light.intensity * phong(normal, viewDirection, lightDirection);
		return  l * attenuation;
	}

	return vec3(0.0f, 0.0f, 0.0f);
}

void main(void)
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 normal = normalize(texelFetch(normalSampler, coord).xyz);
	vec3 position = texelFetch(positionSampler, coord).xyz;

	vec4 materialData = texelFetch(materialSampler, coord);
	material.color = materialData.xyz;
	material.shininess = materialData.w;

	vec3 viewDirection = normalize(cameraPosition - position);
	vec3 positionToLight = light.position - position;
	vec3 lightDirection = normalize(positionToLight);
	float distanceToLight = length(positionToLight);

	fragColor = spotLight(normal, viewDirection, lightDirection, distanceToLight);
}
