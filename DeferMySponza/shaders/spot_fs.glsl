#version 330

#define PI 3.1415926535897932384626433832795f

struct Material 
{
	vec3 albedo;
	float roughness;
	vec3 specular;
} material;

layout(std140) uniform SpotLight
{
	vec3 position;
	float range;
	vec3 direction;
	float cutoff;
	vec3 intensity;
	int castShadow;
} light;

uniform sampler2DRect positionSampler;
uniform sampler2DRect normalSampler;
uniform sampler2DRect materialSampler;
//uniform sampler2DRect materialPropertiesSampler;
uniform sampler2D shadowSampler;

uniform vec3 cameraPosition;
uniform mat4 lightProjViewMatrix;

out vec3 fragColor;

const float spotExponent = 10.0f;
const float shadowBias = 0.00005f;
const int poissonSamples = 4;
const vec2 poissonDisk[4] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760)
);

float shadowMod(vec3 lightSpaceCoord, vec3 normal)
{
	vec3 shadowCoords = lightSpaceCoord.xyz * 0.5f + 0.5f;

	float fragDepth = shadowCoords.z - shadowBias;
	//Poisson Sampling
	float result = 1.0f; //Start in the light
	for (int i = 0; i < poissonSamples; i++)
	{
		vec2 poissonOffset = poissonDisk[i] / 700.0f;
		float shadowDepth = texture(shadowSampler, shadowCoords.xy + poissonOffset).r;
		if (shadowDepth < fragDepth)
			result -= 0.2f;
	}
	return result;
}

float maxdot0(vec3 v, vec3 v2)
{
	return max(dot(v, v2), 0.0f);
}

vec3 lambertianDiffuse(vec3 normal, vec3 lightDirection)
{
	float intensity = maxdot0(normal, lightDirection);
	return material.albedo * light.intensity * intensity;
}

float beckmannDistribution(vec3 normal, vec3 halfVector)
{
	float ndoth = maxdot0(normal, halfVector);
	float r2 = material.roughness * material.roughness;

	float top = exp((pow(ndoth, 2) - 1) / (r2 * pow(ndoth, 2)));
	float bottom = PI * r2 * pow(ndoth, 4);
	return top / bottom;
}

vec3 schlickFresnel(vec3 lightDirection, vec3 halfVector)
{
	float p = pow(1 - maxdot0(lightDirection, halfVector), 5);
	return material.specular + (1 - material.specular) * p;
}

float implicitGeometry(vec3 normal, vec3 halfVector, vec3 viewDirection, vec3 lightDirection)
{
	return maxdot0(normal, lightDirection) * maxdot0(normal, viewDirection);
}

float schlickBeckmannGeometry(vec3 normal, vec3 v)
{
	float k = material.roughness * sqrt(2 / PI);
	float ndotv = dot(normal, v);
	return ndotv / (ndotv * (1 - k) + k);
}

float neumannGeometry(vec3 normal, vec3 lightDirection, vec3 viewDirection)
{
	float ndotl = maxdot0(normal, lightDirection);
	float ndotv = maxdot0(normal, viewDirection);
	return (ndotl * ndotv) / max(ndotl, ndotv);
}

vec3 cookTorranceSpecular(vec3 normal, vec3 halfVector, vec3 viewDirection, vec3 lightDirection)
{
	float distribution = beckmannDistribution(normal, halfVector);
	vec3 fresnel = schlickFresnel(lightDirection, halfVector);
	float geometry = neumannGeometry(normal, lightDirection, viewDirection);

	return (distribution * fresnel * geometry) / (4 * maxdot0(normal, lightDirection) * maxdot0(normal, viewDirection));
}

void main(void)
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 normal = normalize(texelFetch(normalSampler, coord).xyz);
	vec3 position = texelFetch(positionSampler, coord).xyz;

	vec4 materialData1 = texelFetch(materialSampler, coord);
	material.albedo = materialData1.xyz;
	material.roughness = 0.7f;
	material.specular = vec3(0.5f);

	vec3 viewDirection = normalize(cameraPosition - position);
	vec3 positionToLight = light.position - position;
	vec3 lightDirection = normalize(positionToLight);
	float distanceToLight = length(positionToLight);
	vec3 halfDirection = normalize(lightDirection + viewDirection);

	float spot = dot(lightDirection, -light.direction);
	if (spot < light.cutoff)
		return;

	float d = clamp(distanceToLight, 0, light.range);
	float attenuation = (light.range - d) / light.range;
	attenuation *= pow(spot, spotExponent);

	vec3 diffuse = lambertianDiffuse(normal, lightDirection);
	vec3 specular = cookTorranceSpecular(normal, halfDirection, viewDirection, lightDirection);

	vec4 lightSpaceCoord = lightProjViewMatrix * vec4(position, 1.0f);
	float shadowModifier = shadowMod(lightSpaceCoord.xyz / lightSpaceCoord.w, normal);

	fragColor = (diffuse + specular) * attenuation;
	if (light.castShadow == 1)
		fragColor *= shadowModifier;
}
