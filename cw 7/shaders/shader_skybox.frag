#version 430 core

uniform samplerCube skybox;

in vec3 texCoord;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

void main()
{
	FragColor = texture(skybox,texCoord);
	float brightness = dot(FragColor.rgb, vec3(0.2126f, 0.7152f, 0.0722f));
    if(brightness > 0.15f)
        BloomColor = vec4(FragColor.rgb, 1.0f);
    else
        BloomColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}