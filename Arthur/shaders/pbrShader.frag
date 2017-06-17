#version 330 core

// IO parameters
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// Material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// Lights
uniform vec3 lightPos;
uniform vec3 lightColor;

// Camera
uniform vec3 camPos;

// Constants
const float PI = 3.1415926;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float nDoth = max(dot(N,H), 0.0);
	float nDoth2 = nDoth*nDoth;

	float nom = a2;
	float denom = (nDoth2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom/denom;
}

float GeometrySchlickGGX(float nDotv, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r)/8.0;

	float nom = nDotv;
	float denom = nDotv * (1.0-k) + k;

	return nom/denom;
}

float GeometrySmith(vec3 N,vec3 V,vec3 L,float roughness)
{
	float nDotv = max(dot(N,V),0.0);
	float nDotl = max(dot(N,L),0.0);

	float ggx2 = GeometrySchlickGGX(nDotv,roughness);
	float ggx1 = GeometrySchlickGGX(nDotl,roughness);

	return ggx1*ggx2;
}

vec3 FresnelSchlick(float cosTheta,vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta,5.0);
}

void main()
{
	vec3 N = normalize(Normal);
	vec3 V = normalize(camPos - WorldPos);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0,albedo,metallic);

	// Reflectance equation
	vec3 Lo = vec3(0.0);
	for(int i=0; i<4; ++i)
	{
		// Calculate per-light radiance
		vec3 L = normalize(lightPos - WorldPos);
		vec3 H = normalize(V+L);
		float distance = length(lightPos - WorldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColor * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N,H,roughness);
		float G = GeometrySmith(N,V,L,roughness);
		vec3 F = FresnelSchlick(max(dot(H,V),0.0),F0);

		vec3 nominator = NDF * G * F;
		float denominator = 4 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.001; // 0.001 to prevent divide by 0
		vec3 specular = nominator / denominator;

		// kS equal to fresnel
		vec3 kS = F;
		
		// for energy conservation, the diffuse and specular light can't 
		// be above 1.0. To preserve this, the diffuse component 
		// kD should be equal to (1-kS)
		vec3 kD = vec3(1.0) - kS;
		
		// multiply kD by the inverse metalness such that only non metals
		// have diffuse lighting, or a linear blend if partly metal
		kD *= 1.0 - metallic;

		// scale light by N dot L
		float nDotl = max(dot(N,L),0.0);

		// add to outgoing radiance Lo
		Lo += (kD * albedo / PI + specular) * radiance * nDotl;  

	}

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;

	// HDR tonemapping
	color = color / (color + vec3(1.0));
	// gamma correct 
	color = pow(color,vec3(1.0/2.2));

	FragColor = vec4(color,1.0);
}
