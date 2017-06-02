#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform bool ssaoActive;

void main()
{
	// Storing fragment position in first gBuffer texture
	gPosition = FragPos;

	// Storing normal into gBuffer
	gNormal = normalize(Normal);

	if(ssaoActive)
	{
		// And the diffuse per-fragment color
	    gAlbedo.rgb = vec3(0.95);
	}
	else
	{
		// Storing diffuse
		gAlbedo.rgb = texture(texture_diffuse1, TexCoords).rgb;

		// Storing specular
		gAlbedo.a = texture(texture_specular1, TexCoords).r;
	}

}