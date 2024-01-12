#version 430 core

float AMBIENT = 0.1;

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
	vec3 lightDir = normalize(lightDirTS);

	vec3 textureColor = texture2D(colorTexture, vecTex).xyz;

	vec3 N = texture2D(normalSampler, vecTex).xyz;
	N = 2.0 * N - 1.0;
	N = normalize(N);

	//float diffuse=max(0,dot(normal,lightDir));
	float diffuse=max(0,dot(N,lightDir));
	outColor = vec4(textureColor*min(1,AMBIENT+diffuse), 1.0);
}
