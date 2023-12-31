#version 430

//INPUTS
in vec3 clr_from_vtxshader;
in vec4 frag_vtx_normal_pos;


//OUTPUTS
out vec4 clr_from_fragshader; //gl_FragColor

//UNIFORMS
uniform int is_vtx_nrm;
uniform vec4 light_position;
uniform int use_vtx_nrm_clr; //boolean check

void main()
{
	if (!bool(is_vtx_nrm))
	{

		if (!bool(use_vtx_nrm_clr))
		{	
			float ambient = 0.8f;
			float diffuse = 0.6f;
			float NL = max(dot(frag_vtx_normal_pos, light_position), 0.0);
			clr_from_fragshader = vec4(clr_from_vtxshader, 1.f) * ambient + vec4(clr_from_vtxshader, 1.f) * NL * diffuse;
			clr_from_fragshader.w = 1.f;
		}
		else
		{
			clr_from_fragshader = vec4(clr_from_vtxshader, 1.f);
		}
	}
	else
	{
		clr_from_fragshader = vec4(clr_from_vtxshader, 1.0f);
	}
}