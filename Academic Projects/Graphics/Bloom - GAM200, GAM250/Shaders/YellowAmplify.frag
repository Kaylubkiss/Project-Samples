#version 430 core

in vec2 textCoord;
layout (location = 0) out vec4 Color;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D screenTexture; //this isn't actually used as a uniform variable.
uniform vec3 targetColor = vec3(0.0f, 0.72f, 0.54f);



void main()
{	
  vec4 _color = 
  texture(screenTexture, textCoord);

  //if (_color.a > 1.0f)
  //{
   // return;
  //}
  if (_color.r >= 1.0f && _color.g >= 0.0f && _color.b >= 0.00f)
  {
	Color += _color;
	Color.a *= 10.0f;

   	 float brightness = dot(Color.rgb, vec3(0.2126, 0.7152, 0.0722));
    	if(brightness > 1.0)
        BrightColor = vec4(Color.rgb, 1.0);
	else
	BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
  else if (_color.r == 0.0f && _color.g >= 0.5f && _color.b >= 0.0f)
  {
	Color += _color;
	Color.a *= 10.0f;

   	 float brightness = dot(Color.rgb, vec3(0.2126, 0.7152, 0.0722));
    	if(brightness > 1.0f)
        BrightColor = vec4(Color.rgb, 1.0);
	else
	BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
   }
}