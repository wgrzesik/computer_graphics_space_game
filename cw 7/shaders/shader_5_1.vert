#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 transformation;
uniform mat4 modelMatrix;

out vec3 worldPos;
out vec2 vecTex;

uniform vec3 lightPos;
uniform vec3 cameraPos;

out vec3 viewDirTS;
out vec3 lightDirTS;
out mat3 TBN;


void main()
{
	vec3 tangent = normalize(mat3(modelMatrix) * vertexTangent);
	vec3 bitangent = normalize(mat3(modelMatrix) * vertexBitangent);
	vec3 normal = normalize(mat3(modelMatrix) * vertexNormal);

	mat3 TBN = transpose(mat3(tangent, bitangent, normal));

	vec3 worldPos = (modelMatrix * vec4(vertexPosition, 1)).xyz;

	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 lightDir = normalize(lightPos - worldPos);

	viewDirTS = vec3(TBN * viewDir);
    lightDirTS = vec3(TBN * lightDir);

	vecTex = vec2((worldPos.x + 10.0) / 20.0, 1.0 - (worldPos.y + 10.0) / 20.0);

	gl_Position = transformation * vec4(vertexPosition, 1.0);
}