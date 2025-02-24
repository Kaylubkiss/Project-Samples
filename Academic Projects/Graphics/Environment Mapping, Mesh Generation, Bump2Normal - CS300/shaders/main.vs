#version 330 core

/*  These vertex attributes are in model space */
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec3 tan;
layout (location = 3) in vec3 bitan;
layout (location = 4) in vec2 uv;

uniform mat4 mvMat;     /*  model-view matrix for positions */
uniform mat4 nmvMat;    /*  model-view matrix for normals */
uniform mat4 projMat;   /*  projection matrix */

uniform bool lightOn;           /*  whether lighting should be applied */
uniform int  numLights;
uniform vec3 lightPosVF[10];    /*  light pos already in view frame */

uniform bool normalMappingOn;   /*  whether normal mapping should be applied */


out vec2 uvCoord;

/*  Output vectors:
    - If normalMapping is on then these vectors are in tangent space.
    - Otherwise they are in view space
*/
out vec3 lightDir[10];
out vec3 viewDir;
out vec3 normal;


void main(void) 
{
    vec4 posVF = mvMat * vec4(pos, 1.0);

    /*    For object transformation */
    gl_Position = projMat * posVF;

    /*  For object texturing */
    uvCoord = uv;


    if (lightOn)
    {
        //@TODO Assignment4
        /*  Compute view direction, light direction and normal in view space */
        /*  ... */
        normal = vec3(nmvMat * vec4(nrm,0));
        
        viewDir = -posVF.xyz;

        for (int i = 0; i < numLights; ++i)
        {
            lightDir[i] = lightPosVF[i] - posVF.xyz;
        }


        if (normalMappingOn)    
        {
            //@TODO Assignment4
            /*  If normal mapping is used, bring view dir and light dir to tangent space. 
                To do this, the TBN matrix needs to be used.
                Normal doesn't need to be transformed since we'll obtain it directly
                from the normal texture in the fragment shader.
            */
            /*  ... */

            vec3 tang = normalize(vec3(mvMat * vec4(tan,0)));
            vec3 bitang = normalize(vec3(mvMat * vec4(bitan,0)));
            vec3 norm = normalize(normal);

            mat3 TBN = transpose(mat3(tang,bitang,norm));

            for (int i = 0; i < numLights; ++i)
            {
               lightDir[i] = TBN * lightDir[i];
            }

            viewDir = TBN * viewDir;

        }
    }
}