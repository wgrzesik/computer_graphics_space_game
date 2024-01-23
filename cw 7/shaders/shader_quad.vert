#version 430 core

layout(location = 0) in vec3 vertexPosition;


out vec2 vecTex;

void main()
{
	//worldPos = (modelMatrix* vec4(vertexPosition,1)).xyz;
	vecTex = vertexPosition.xy*0.5+0.5;
	gl_Position = vec4(vertexPosition, 1.0);
}
