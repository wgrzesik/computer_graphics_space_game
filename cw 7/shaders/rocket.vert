#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

out vec3 fragNormal;
out vec2 fragTexCoord;

uniform mat4 transformation;
uniform mat4 modelMatrix;

void main()
{
    gl_Position = transformation * modelMatrix * vec4(inPosition, 1.0);
    fragNormal = mat3(transpose(inverse(modelMatrix))) * inNormal;
    fragTexCoord = inTexCoord;
}