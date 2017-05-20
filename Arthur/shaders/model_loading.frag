#version 330 core

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light
{
	//vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;

out vec4 color;

uniform Material material;
uniform Light light;
uniform sampler2D texture_diffuse1;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightIntensity;
uniform vec3 viewPos;
uniform int lightMode;

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 lightDir;
    // float distance, attenuation;
    if(lightMode == 1)
    {
    	lightDir = normalize(lightPos - FragPos);
    }
    else
    {
    	lightDir = normalize(-light.direction);
    }
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    // Attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// Ambient 
    vec3 ambient = material.ambient * lightColor;

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * material.diffuse) * lightColor;

    // Specular
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = material.specular * spec * lightColor;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Final result
    vec3 result = (ambient + diffuse + specular);
    color = vec4(result, 1.0);
}