#version 430 core


uniform sampler2D colorTexture;
uniform sampler2D highlightTexture;

in vec2 vecTex;

out vec4 outColor;
void main()
{
	vec3 color = texture(colorTexture,vecTex).rgb;
	vec3 color2 = texture(highlightTexture,vecTex).rgb;
	outColor = vec4(color+color2, 1.0);
}
