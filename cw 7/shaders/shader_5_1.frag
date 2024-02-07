#version 430 core

float AMBIENT = 0.2;

uniform vec3 color;
uniform float metalness;
uniform float roughness;
uniform vec3 lightPos;
uniform vec3 lightColor;
in vec3 viewDirTS;
in vec3 lightDirTS;
in vec3 worldPos;

out vec4 outColor;
in mat3 TBN;
void main()
{
    vec3 normal = normalize(TBN * vec3(0, 0, 1)); // Normal z TBN
    vec3 L = normalize(lightPos - worldPos);
    vec3 V = normalize(viewDirTS);
    vec3 H = normalize(L + V); 

    float k = pow((roughness + 1), 2.0) / 8.0;

    float D = (roughness * roughness) / (3.14159 * pow(pow(dot(normal, H), 2.0) * (roughness * roughness - 1.0) + 1.0, 2.0));
    float G = dot(normal, L) / (dot(normal, L) * (1.0 - k) + k);
    vec3 F0 = mix(vec3(0.04), color, metalness);
    vec3 F = F0 + (1.0 - F0) * pow(1 - dot(V, H), 5.0);

    vec3 specular = lightColor * (D * G * F) / (4 * dot(normal, L) * dot(normal, V) + 0.00001);
    vec3 kD = vec3(1.0) - F;

    vec3 diffuse = lightColor * kD * (color / 3.1458493);
    
    vec3 finalColor = (diffuse + specular) * min(1.0, AMBIENT + max(dot(normal, L), 0.0));
  
    outColor = vec4(finalColor, 1.0);
}