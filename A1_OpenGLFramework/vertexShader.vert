#version 430

//INPUTS
layout(location = 0) in vec3 pos_from_vtxbuffer; //position
layout(location = 1) in vec3 clr_from_vtxbuffer; //color
layout (location = 3) in vec3 vtx_normal_pos; //vertex normals 
layout (location = 4) in vec3 vtx_normal_clr; //vertex normal colors

//OUTPUTS
smooth out vec3 clr_from_vtxshader; //output color to fragment shader
out vec4 frag_vtx_normal_pos; //vertex normal position (used to calculate light)

//UNIFORM MATRICES
uniform mat4 uMVP; //model-view-projection;
uniform mat4 normalMatrix;

//UNIFORM VARIABLES
uniform vec3 vtx_nrm_clr; //color of vertex normal
uniform int is_vtx_nrm; //boolean check
uniform int use_vtx_nrm_clr; //boolean check

void main()
{
		gl_Position = uMVP * vec4(pos_from_vtxbuffer, 1.f);

		frag_vtx_normal_pos = normalMatrix * vec4(pos_from_vtxbuffer, 1.f); 
		
		if (!bool(is_vtx_nrm))
		{
			if (!bool(use_vtx_nrm_clr))
			{
				clr_from_vtxshader = clr_from_vtxbuffer;
			}
			else
			{
				clr_from_vtxshader = vtx_normal_clr;
			}
		}
		else
		{
			clr_from_vtxshader = vtx_nrm_clr; 
		}
}