#version 330 core

uniform sampler2D colorTex;     /*  Base color texture */
uniform sampler2D normalTex;    /*  Normal texture for normal mapping */
uniform sampler2D bumpTex;      /*  Bump texture for bump mapping */

in vec2 uvCoord;

uniform bool lightOn;
uniform int numLights;
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform int specularPower;

/*  These could be in view space or tangent space */
in vec3 lightDir[10];
in vec3 viewDir;
in vec3 normal;


uniform bool normalMappingOn;   /*  whether normal mapping is on */
uniform bool parallaxMappingOn; /*  whether parallax mapping is on */

out vec4 fragColor;

/******************************************************************************/
/*!
\fn     void BlinnPhong()
\brief
        Calculate Lighting with Blinn-Phong calculation.
*/
/******************************************************************************/
void BlinnPhong()
{

    vec4 V = normalize(vec4(viewDir,0));
    
    vec2 texCoords = uvCoord;

    if (parallaxMappingOn)
    {

        float scale = .15f;
        float bias = -.005f;
        float h = texture(bumpTex, uvCoord).r ;

        float h_sb = h * scale + bias;

        texCoords = uvCoord + ((h_sb) * V.xy);
       
    }
    
    /*  Compute lighting using Phong equation */

    vec4 Clight = vec4(texture(colorTex, texCoords).rgb, 1.0);
    
    vec4 N = vec4(0);

    if (!normalMappingOn)
    {
        N = normalize(vec4(normal, 0.0f));
    }
    else
    {   
       N = normalize(vec4(texture(normalTex, texCoords).rgb * 2.0 - 1.f, 0.0));
    }
   
    vec4 diffuse_accum = vec4(0,0,0,0);
    vec4 specular_accum = vec4(0,0,0,0);
    
    for (int i = 0; i < numLights; ++i)
    {
       
            vec4 diffuse_portion = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    
            vec4 specular_portion = vec4(0.0f, 0.0f, 0.0f, 0.0f);
           
            vec4 L = normalize(vec4(lightDir[i], 0));
          

            float m_dot_L = dot(N, L);

            //////        equation portions  /////////
    
            if (m_dot_L > 0.0f)
            { 
                diffuse_portion = diffuse * m_dot_L; 
            }

            /// Use halfway vector to calculate specular light/////
            vec4 H = normalize(L + V);
            
            float N_dot_H = dot(N, H);
            
            if (N_dot_H > 0.0f)
            {
                specular_portion = 
                specular * pow(N_dot_H, specularPower);
            }

            diffuse_accum += diffuse_portion;
            specular_accum += specular_portion;
      }
        
      fragColor = (ambient + diffuse_accum + specular_accum) * Clight;

      fragColor.a = 1;
}

void main(void)
{
    if (!lightOn)
    {
        fragColor = vec4(texture(colorTex, uvCoord).rgb, 1.0);
        return;
    }

    //@TODO Assignment4
    /*  If normal mapping is off, then use the interpolated normal from vertex shader.
        Otherwise, sample the normal from normal map.

        If parallax mapping is off, then uvCoord should be used for all the texture sampling.
        Otherwise, we should use the adjusted uvCoord.
        Adjusted uvCoord is set to be uvCoord + h * viewDir.xy,
        where h is height from bump map * 0.15 - 0.005.

        For lighting, Blinn-Phong model should be used.
    */
    /*  ... */


    //Calculate Lighting

    BlinnPhong();

      
}