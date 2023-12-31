#version 430 core

in vec2 textCoord;

out vec4 FragColor;


uniform sampler2D usampler;

uniform float exposure = 0.8f;

void main()
{
   // const float gamma = 2.4f; //special number

    vec3 hdrColor = texture(usampler, textCoord).rgb;
  	
    // exposure tone mapping
   // vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
  //  mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(hdrColor * exposure, 1.0);
}