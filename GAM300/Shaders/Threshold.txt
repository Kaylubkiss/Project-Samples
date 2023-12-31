#version 430 core

in vec2 textCoord;
out vec4 FragColor;

uniform sampler2D usampler;


uniform float threshold = 0.625;
uniform float haze = 0.00;

void main()
{
   FragColor = texture(usampler, textCoord);

   float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    brightness = pow(brightness, 3);
    FragColor.rgb *= brightness * haze + max(0, brightness - threshold) * (1 - haze);


}