#version 430 core

float AMBIENT = 0.3;
float roughness = 0.2;
float metalic = 0.8;


uniform vec3 color;
uniform sampler2D colorTexture;
uniform sampler2D normalSampler;

in vec3 worldPos;
in vec2 vecTex;

in vec3 viewDirTS;
in vec3 lightDirTS;

out vec4 outColor;

void main()
{
	vec3 normal = vec3(0,0,1);
	vec3 L = normalize(lightDirTS- worldPos);
	vec3 V = normalize(viewDirTS - worldPos);

	vec3 textureColor = texture2D(colorTexture, vecTex).xyz;

	vec3 N = texture2D(normalSampler, vecTex).xyz;
	N = 2.0 * N - 1.0;
	N = normalize(N);

	vec3 H = normalize(L + V); // mo¿e trzeba zmieniæ plus na minus
    float NdotH = max(0.0, dot(N, H)); // zamieniæ kolejnoœæ i 00000001 
    float NdotL = max(0.0, dot(N, L));
    float NdotV = max(0.0, dot(N, V));
    float VdotH = max(0.0, dot(V, H));
	float k = pow((roughness +1),2.0)/8.0;


	float D = (roughness * roughness) / (3.14159 * pow(pow(NdotH * NdotH,2.0) * (roughness * roughness - 1.0) + 1.0, 2.0));
	float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
	vec3 F0 = vec3(0.04); 
    float G = ggx1 * ggx2;
	vec3 F = F0 + (1.0-F0)*pow(1-VdotH,5.0);

	vec3 specular = (D*G*F)/(4*NdotL*NdotV);
	vec3 kD = vec3(1.0) - F;

	vec3 BRDF = kD*(textureColor/3.1458493) + specular;





	float diffuse=max(0,dot(N,L));

	outColor = vec4(( textureColor * min(1,AMBIENT+diffuse)), 1.0);
}
