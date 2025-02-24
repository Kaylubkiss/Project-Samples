#version 430 core

in vec2 textCoord;
out vec4 Color;

uniform sampler2D screenTexture; //this isn't actually used as a uniform variable.
uniform vec3 targetColor = vec3(0.0f, 0.72f, 0.54f);



void main()
{	
   vec3 grey_color = vec3(0.5f, 0.0f, 0.5f);
   vec4 _color = 
   texture(screenTexture, textCoord);

   
   if (_color.rgb == vec3(0.0f, 0.0f, 0.0f) ||
    _color.rgb == vec3(1.0f, 1.0f, 1.0f))
   {
   	Color = _color;
	Color.a = _color.a * 15.0f;
   }
   else if (abs(_color.rgb - targetColor).b <= 0.1f &&
   abs(_color.rgb - targetColor).r <= 0.1f &&
   abs(_color.rgb - targetColor).g <= 0.1f)
   {
	Color = _color;
	Color.b = 1.0f;
	
   }
   else
   {
	Color = _color;
	float r = Color.r;
	Color.r = Color.b;
	Color.b = r;
	Color.a = 1.0f;
   }	
   
}