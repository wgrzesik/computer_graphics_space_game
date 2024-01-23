#version 430 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

float AMBIENT = 0.1;

uniform vec3 color;
uniform sampler2D colorTexture;
uniform sampler2D normalSampler;

in vec3 worldPos;
in vec2 vecTex;

in vec3 viewDirTS;
in vec3 lightDirTS;


void main()
{
	vec3 lightDir = normalize(lightDirTS);
	vec3 textureColor = texture2D(colorTexture, vecTex).xyz;

	vec3 N = texture2D(normalSampler, vecTex).xyz;
	N = 2.0 * N - 1.0;
	N = normalize(N);

	float diffuse=max(0, dot(N, lightDir));
	FragColor = vec4(textureColor * min(1, AMBIENT + diffuse), 1.0);


	float brightness = dot(FragColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
    if(brightness > 0.15f)
        BloomColor = vec4(FragColor.rgb, 1.0f);
    else
        BloomColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);


}
