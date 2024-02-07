#version 430 core


uniform float exposition;
uniform sampler2D sunTexture;
out vec4 outColor;
in vec2 TexCoords;

void main()
{
	vec3 textureColor = texture2D(sunTexture, TexCoords).xyz;
	vec3 adjustedColor = 1.0 - exp(-textureColor * exposition);
    
    // Output the color with unchanged alpha
    outColor = vec4(clamp(adjustedColor, 0.0, 1.0), 1.0);
	//outColor = vec4(textureColor*min(1,exposition), 1.0);
}
