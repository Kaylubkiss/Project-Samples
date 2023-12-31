#version 430 core

in vec2 textCoord;

out vec4 FragColor;

const int kernelSize = 10;

uniform sampler2D usampler;

//gaussian distribution values
//const float weights[kernelSize] = 
//{
//	0.2270270270, 
//	0.1945945946, 
//	0.1216216216,
// 0.0540540541, 
//	0.0162162162	
//};

const float weights[kernelSize] = 
{
  0.22697126013264554,
  0.19454679439941047,
  0.12159174649963155,
  0.05404077622205846,
  0.0302, //added -- completely uncalculated
  0.0200, //added -- completely uncalculated
  0.0170, //added
  //0.0166, //added
  0.016212232866617538,
  0.0141, //added
  0.0107 //added
};


//this is on a 5x5 kernel size

void main()
{
	
	vec2 offset = 1.0f / textureSize(usampler, 0);

	FragColor.rgb = texture(usampler, textCoord).rgb * weights[0];


	for (int i = 1; i < kernelSize; ++i)
	{
		FragColor.rgb += texture(usampler, textCoord + vec2(0.0, offset.y * i)).rgb * weights[i];

		FragColor.rgb += texture(usampler, textCoord - vec2(0.0, offset.y * i)).rgb * weights[i];
	}

	FragColor.a = 1.0f;
}