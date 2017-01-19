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
//uniform sampler2DRect materialPropertiesSampler;
uniform sampler2D shadowSampler;

uniform vec3 cameraPosition;
uniform mat4 lightProjViewMatrix;

out vec3 fragColor;

const float spotExponent = 0.0f;
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

	return material.color * diffuseIntensity + vec3(1.0f, 1.0f, 1.0f) * specularIntensity;
}

vec3 spotLight(vec3 normal, vec3 viewDirection, vec3 lightDirection, float distanceToLight)
{
	float spot = dot(lightDirection, -light.direction);
	if (spot > light.cutoff)
	{
		float d = clamp(distanceToLight, 0, light.range);
		float attenuation = (light.range - d) / light.range;
		attenuation *= pow(spot, spotExponent);

		vec3 l = light.intensity * phong(normal, viewDirection, lightDirection);
		return  l * attenuation;
	}

	return vec3(0.0f, 0.0f, 0.0f);
}

vec3 lambertianDiffuse(vec3 normal, vec3 lightDirection)
{
	float intensity = max(dot(normal, lightDirection), 0.0f);
	return material.color * intensity;
}

vec3 fresnelSchlick(float cosTheta, vec3 reflectance)
{
	return reflectance + (1.0f - reflectance) * pow(1.0f - cosTheta, 5.0f);
}

float distribution()
{
	return 0;
}

vec3 cookTorranceSpecular()
{
	return vec3(0);
}

void main(void)
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 normal = normalize(texelFetch(normalSampler, coord).xyz);
	vec3 position = texelFetch(positionSampler, coord).xyz;

	/*material.albedo = texelFetch(materialAlbedoSampler, coord).rgb;
	vec4 props = texelFetch(materialPropertiesSampler, coord);
	material.metallic = props.r;
	material.smoothness = props.g;*/

	vec4 materialData = texelFetch(materialSampler, coord);
	material.color = materialData.rgb;
	material.shininess = materialData.w;

	vec3 viewDirection = normalize(cameraPosition - position);
	vec3 positionToLight = light.position - position; //Wi
	vec3 lightDirection = normalize(positionToLight);
	float distanceToLight = length(positionToLight);

	vec4 lightSpaceCoord = lightProjViewMatrix * vec4(position, 1.0f);
	float shadowModifier = shadowMod(lightSpaceCoord.xyz / lightSpaceCoord.w, normal);

	//vec3 reflectance = vec3(0.04f);
	//reflectance = mix(reflectance, material.albedo, material.metallic);
	////Amount of reflected light
	//vec3 fresnel = fresnelSchlick(max(dot(normal, viewDirection), 0.0f), reflectance);

	//vec3 ks = fresnel;
	//vec3 kd = vec3(1.0f) - ks;

	fragColor = spotLight(normal, viewDirection, lightDirection, distanceToLight) * shadowModifier;
}
