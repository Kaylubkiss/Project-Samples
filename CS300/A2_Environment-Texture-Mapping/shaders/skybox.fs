#version 330 core

uniform samplerCube texCube;

in vec3 texCoord;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(texCube, texCoord);
}