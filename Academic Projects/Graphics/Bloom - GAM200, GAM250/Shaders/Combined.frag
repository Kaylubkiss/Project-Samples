#version 430 core

in vec2 textCoord;

out vec4 FragColor;


layout(binding = 0) uniform sampler2D screenTexture1;
layout(binding = 1) uniform sampler2D screenTexture2;

//this is on a 5x5 kernel size

void main()
{
	
	FragColor.rgb = texture(screenTexture1, textCoord).rgb;
	FragColor.rgb += texture(screenTexture2, textCoord).rgb;
	FragColor.a = 1.0f;
}