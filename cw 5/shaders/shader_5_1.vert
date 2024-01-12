#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

uniform mat4 transformation;

out vec3 FragNormal;
uniform mat4 modelMatrix;
uniform vec3 cameraPos;

out vec3 viewDir;
out vec4 vertexPosWorld;

void main()
{
	gl_Position = transformation * vec4(vertexPosition, 1.0);
	
	vec4 transformedNormal = modelMatrix * vec4(vertexNormal, 0.0);
    FragNormal = normalize(transformedNormal.xyz);

	vertexPosWorld = modelMatrix * vec4(vertexPosition, 1.0);

	

}
