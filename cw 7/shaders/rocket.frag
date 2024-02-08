#version 330 core

in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 fragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    // Dodaj inne właściwości materiałów, jeśli są potrzebne
};

uniform Material material;
uniform vec3 lightPosition;
uniform vec3 viewPosition;

void main()
{
    vec3 normal = normalize(fragNormal);
    vec3 lightDirection = normalize(lightPosition - gl_FragCoord.xyz);
    float diff = max(dot(normal, lightDirection), 0.0);
    
    vec3 viewDirection = normalize(viewPosition - gl_FragCoord.xyz);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0);

    vec3 ambient = material.ambient;
    vec3 diffuse = material.diffuse * diff;
    vec3 specular = material.specular * spec;

    vec3 finalColor = ambient + diffuse + specular;
    
    fragColor = vec4(finalColor, 1.0);
}