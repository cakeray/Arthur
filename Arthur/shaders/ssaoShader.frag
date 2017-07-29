#version 330 core
out float FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform int num_samples;
uniform vec3 samples[64];
uniform mat4 projection;

uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform int power;

const vec2 noiseScale = vec2(1280.0/4.0 , 720.0/4.0);

void main()
{
	vec3 fragPos = texture(gPosition,TexCoords).xyz;
	vec3 normal = texture(gNormal, TexCoords).rgb;
	vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

	vec3 tangent = normalize(randomVec - normal * dot(randomVec,normal));
	vec3 biTangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, biTangent, normal);

	float occlusion = 0.0;
	for(int i = 0 ; i < kernelSize; i++)
	{
		vec3 sample = TBN * samples[i];
		sample = fragPos + sample * radius;

		vec4 offset = vec4(sample, 1.0);
		offset = projection * offset;
		offset.xyz/=offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = texture(gPosition, offset.xy).z;

		float rangeCheck = smoothstep(0.0,1.0,radius/abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion/kernelSize);
	FragColor = pow(occlusion,power);
}