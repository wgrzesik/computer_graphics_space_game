#version 430 core

uniform vec3 color;
out vec4 outColor;

uniform vec3 lightColor;
uniform vec3 cameraPos;

uniform vec3 lightPos;
in vec3 FragNormal;
in vec4 vertexPosWorld;

uniform float exposition;

void main()
{
	vec3 viewDir = normalize(cameraPos - vertexPosWorld.xyz);
    vec3 normalizedNormal = normalize(FragNormal);

    vec3 lightDir = normalize(lightPos - vertexPosWorld.xyz);
   
    float diffuse = max(dot(normalizedNormal, lightDir), 0.0);

  
    vec3 normalizedViewDir = normalize(viewDir);
    vec3 reflectDir = reflect(-normalize(lightDir), normalizedNormal);
    float specular = pow(max(dot(normalizedViewDir, reflectDir), 0.0), 8.0);

    //zad_6
    float distance = length(lightPos - vertexPosWorld.xyz);
    vec3 newLightColor = lightColor / distance;
    vec3 finalColor = newLightColor * (color * diffuse+specular);

    finalColor = 1.0 - exp(-finalColor * exposition);

    outColor = vec4(finalColor, 1.0);
	
}
