#version 330 core

uniform samplerCube texCube;

uniform mat4 viewMat;

uniform int sphereRef;

in vec3 view, normal;


out vec4 fragColor;


void main(void)
{
    /*  Compute reflected/refracted vectors in view frame for higher accuracy, 
        then transform them back to world frame for texture sampling.

        If sphereRef = 0 then there's only reflection.
        If sphereRef = 1 then there's only refraction.
        If sphereRef = 2 then there are both reflection and refraction.
        In that case, assume that reflection accounts for 70% of the color.
		
		The refractive index for the sphere is 1.5. For simplicity, we also
		assume that each ray going into the sphere is refracted only once.
    */
    vec3 nrmView = normalize(view);
    vec3 normal = normalize(normal);

    vec3 reflectedVector = normalize(vec3(inverse(viewMat) * vec4(reflect(nrmView, normal),0)));
    vec3 refractedVector = vec3(inverse(viewMat) * vec4(refract(nrmView, normal, 1.0 / 1.5),0)); // Refractive index of 1.5

    vec3 finalColor = vec3(0.0); // Initialize final color
       
    if (sphereRef == 0) {
        // Only reflection
        finalColor += texture(texCube, reflectedVector).rgb;
    } else if (sphereRef == 1) {
        // Only refraction
        finalColor += texture(texCube, refractedVector).rgb;
    } else if (sphereRef == 2) {
        // Both reflection and refraction
        vec3 reflectedColor = texture(texCube, reflectedVector).rgb;
        vec3 refractedColor = texture(texCube, refractedVector).rgb;

        finalColor += 0.7 * reflectedColor + 0.3 * refractedColor;
    }

    fragColor = vec4(finalColor, 1.0);
}