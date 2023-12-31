#version 330 core

uniform mat4 viewMat;

out vec3 texCoord;

void main(void)
{
    /*  Render background using a full-screen quad defined as the far plane in NDC frame (+z).

        This quad texture coordinates are defined in world-frame as the 4 back corners 
        of a 2x2x2 box symmetric about the world origin.
    */

    // Define the 4 back corners of the 2x2x2 box in world coordinates
    const vec3 screenQuad[4] = vec3[4] (
    
        vec3(-1.0f,-1.0f, -1.0f), //bottom left
        vec3( 1.0f,-1.0f, -1.0f), //top  left
        vec3( -1.0f, 1.0f, -1.0f), //top right
        vec3(1.0f, 1.0f, -1.0f) //bottom right

    );

    gl_Position = vec4(screenQuad[gl_VertexID].xy, 1.0, 1.0);

    texCoord = transpose(mat3(viewMat)) * vec3(screenQuad[gl_VertexID].xyz);
}