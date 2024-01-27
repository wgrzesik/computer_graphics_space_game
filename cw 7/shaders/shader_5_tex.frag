#version 430 core

float AMBIENT = 0.6;
float roughness;
float metalic;


uniform vec3 color;
uniform sampler2D colorTexture;
uniform sampler2D normalSampler;

uniform sampler2D metalnessTexture;
uniform sampler2D roughnessTexture;

in vec3 worldPos;
in vec2 vecTex;

in vec3 viewDirTS;
in vec3 lightDirTS;

out vec4 outColor;

void main()
{
	vec3 normal = vec3(0,0,1);
	vec3 L = (lightDirTS);
	vec3 V = (viewDirTS);

	vec3 textureColor = texture2D(colorTexture, vecTex).xyz;
	float metalnessValue = texture2D(metalnessTexture, vecTex).r;
    float roughnessValue = texture2D(roughnessTexture, vecTex).r;

	roughness =roughnessValue;
	metalic = metalnessValue;

	vec3 N = texture2D(normalSampler, vecTex).xyz;
	N = 2.0 * N - 1.0;
	N = normalize(N);

	vec3 H = normalize(L + V); 
    float NdotH = max(0.0, dot(N, H)); 
    float NdotL = max(dot(N, L),0.0000001 );
    float NdotV = max(0.0, dot(N, V));
    float VdotH = max(0.00001, dot(V, H));
	float k = pow((roughness +1),2.0)/8.0;


	float D = (roughness * roughness) / (3.14159 * pow(pow(NdotH * NdotH,2.0) * (roughness * roughness - 1.0) + 1.0, 2.0));
	float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
	float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
	vec3 F0 = mix(vec3(0.04), vec3(1.0), metalic); 
    float G = ggx1 * ggx2;
	vec3 F = F0 + (1.0-F0)*pow(1-dot(V,H),5.0);

	vec3 specular = (D*G*F)/(4*NdotL*NdotV+0.00001);
	vec3 kD = vec3(1.0) - F;

	vec3 BRDF = kD*(textureColor/3.1458493) + specular;





	float diffuse=max(0.0001,dot(N,L));
	vec3 lambertian = max(0.00001, dot(N,L))*textureColor;

	vec3 Final = (kD*textureColor/3.1458993) + specular;
	outColor = vec4(Final*min(1.0,AMBIENT + diffuse), 1.0);
}
