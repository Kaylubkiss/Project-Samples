/******************************************************************************/
/*!
\file   graphics.cpp
\par    Purpose: Implement the graphics-related functions
\par    Language: C++
\par    Platform: Visual Studio 2013, Windows 7 64-bit
\author Sang Le, YOUR_NAME
\par    Email: sang.le\@digipen.edu, YOUR_EMAIL_ID
\par    DigiPen login: sang.le, YOUR_LOGIN_ID
\date   YOUR_COMPLETION_DATE
*/
/******************************************************************************/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "graphics.hpp"
#include "textfile.h"
#include "image_io.h"

#include <iostream>
#include <ctime>


/******************************************************************************/
/*  Graphics-related variables                                                */
/******************************************************************************/

/*  For displaying FPS */
clock_t currTime, prevTime;
int frameCount;
float secCount;        /*  Num of seconds from prevTime to currTime */
float fps;

/*  For clearing depth buffer */
GLfloat one = 1.0f;


/*  For color texture */
GLuint texID[ImageID::NUM_IMAGES];
const char *objTexFile[ImageID::NUM_IMAGES] = { "../images/stone.png", "../images/wood.png", "../images/pottery.png" };

/*  For bump/normal texture */
const char *bumpTexFile = "../images/stone_bump.png";
GLuint bumpTexID, normalTexID;

/*  For activating the texture ID. We need these 3 separate IDs because 
    they are used at the same time for the base 
*/
struct ActiveTexID
{
    enum { COLOR = 0, NORMAL, BUMP };
};


/*  For environment texture */
const char *skyboxTexFile = "../images/skybox.png";
GLuint skyboxTexID;
int skyboxFaceSize;

/*  6 faces of the texture cube */
struct CubeFaceID
{
    enum { RIGHT = 0, LEFT, TOP, BOTTOM, BACK, FRONT, NUM_FACES };
};


/*  For generating sphere "reflection/refraction" texture */
GLuint sphereTexID;

/*  Toggling sphere reflection/refraction */
int sphereRef = RefType::REFLECTION_ONLY;


/*  For generating mirror "reflection" texture */
GLuint mirrorTexID, mirrorFrameBufferID;

/*  For turning off generating mirror "reflection" texture when mirror is not visible */
bool mirrorVisible;


/*  Toggling parallax mapping */
bool parallaxMappingOn = false;


/*  Matrices for view/projetion transformations */
/*  Viewer camera */
Mat4 mainCamViewMat, mainCamProjMat, mainCamMVMat[ObjID::NUM_OBJS], mainCamNormalMVMat[ObjID::NUM_OBJS];

/*  Mirror camera */
Mat4 mirrorCamViewMat, mirrorCamProjMat, mirrorCamMVMat[ObjID::NUM_OBJS], mirrorCamNormalMVMat[ObjID::NUM_OBJS];

/*  Sphere cameras - we need 6 of them to generate the texture cubemap */
Mat4 sphereCamViewMat[CubeFaceID::NUM_FACES], sphereCamProjMat, sphereCamMVMat[CubeFaceID::NUM_FACES][ObjID::NUM_OBJS], sphereCamNormalMVMat[CubeFaceID::NUM_FACES][ObjID::NUM_OBJS];


/*  For indicating which pass we are rendering, since we will use the same shaders to render the whole scene */
/*  SPHERETEX_GENERATION: Generating the scene texture for the sphere reflection/refraction */
/*  MIRRORTEX_GENERATION: Generating the scene texture for the mirror reflection */
/*  NORMAL              : Render the final scene as normal */
struct RenderPass
{
    enum { SPHERETEX_GENERATION, MIRRORTEX_GENERATION, NORMAL };
};


/*  We need 3 set of shaders programs */
/*  MAIN_PROG   : Render all the objects in the scene, used for the above 3 passes */
/*  SKYBOX_PROG : Render the background */
/*  SPHERE_PROG : Render the relective/refractive sphere */
struct ProgType
{
    enum { MAIN_PROG = 0, SKYBOX_PROG, SPHERE_PROG, NUM_PROGTYPES };
};

struct ShaderType
{
    enum { VERTEX_SHADER = 0, FRAGMENT_SHADER, NUM_SHADERTYPES };
};

/*  Shader filenames */
const char file[ProgType::NUM_PROGTYPES][ShaderType::NUM_SHADERTYPES][100] =
{
    {
        { "../shaders/main.vs" },
        { "../shaders/main.fs" }
    },
    {
        { "../shaders/skybox.vs" },
        { "../shaders/skybox.fs" }
    },
    {
        { "../shaders/sphere.vs" },
        { "../shaders/sphere.fs" }
    }
};

/*  ID of the shader programs that we'll use */
GLuint prog[ProgType::NUM_PROGTYPES];



/*  Locations of the variables in the shader. */
/*  Locations of transform matrices */
GLint mainMVMatLoc, mainNMVMatLoc, mainProjMatLoc;  /*  used for main program */
GLint skyboxViewMatLoc;                             /*  used for skybox program */
//GLint skyboxProjMatLoc;                             //STUDENT ADDED
GLint sphereMVMatLoc, sphereNMVMatLoc, sphereProjMatLoc, sphereViewMatLoc;  /*  used for sphere program */

/*  Location of color textures */
GLint textureLoc;                       /*  Normal object texture */
GLint sphereTexCubeLoc;                 /*  Texture cubemap for the sphere reflection/refraction */
GLint skyboxTexCubeLoc;                 /*  Texture cubemap for the skybox background rendering */

GLint sphereRefLoc;                     /*  For sending reflection/refraction status */

/*  Location of bump/normal textures */
GLint normalTexLoc, bumpTexLoc;

/*  For indicating whether object has normal map, and parallax mapping status */
GLint normalMappingOnLoc, parallaxMappingOnLoc;

/*  Location of light data */
GLint numLightsLoc, lightPosLoc;
GLint lightOnLoc;
GLint ambientLoc, diffuseLoc, specularLoc, specularPowerLoc;



/******************************************************************************/
/*!
\fn     void ValidateShader(GLuint shader, const char *file)
\brief
        Check whether shader files can be compiled successfully.
\param  shader
        ID of the shader
\param  file
        Shader file name
*/
/******************************************************************************/
void ValidateShader(GLuint shader, const char *file)
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];

    GLsizei length = 0;
    GLint result;

    glGetShaderInfoLog(shader, 512, &length, buffer);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result != GL_TRUE && length > 0)
    {
        std::cerr << "Shader " << file << " compilation error: " << buffer << "\n";
        exit(1);
    }
    else
        std::cout << "Shader " << file << " compilation successful.\n";
}


/******************************************************************************/
/*!
\fn     void ValidateProgram(GLuint program)
\brief
        Check whether shader program can be linked successfully.
\param  program
        ID of the shader program
*/
/******************************************************************************/
void ValidateProgram(GLuint program)
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;
    GLint status;

    /*  Ask OpenGL to give us the log associated with the program */
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer); 
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status != GL_TRUE && length > 0)
    {
        std::cerr << "Program " << program << " link error: " << buffer << "\n";
        exit(1);
    }
    else
        std::cout << "Program " << program << " link successful.\n";

    /*  Ask OpenGL to validate the program */
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        std::cerr << "Error validating shader " << program << ".\n";
        exit(1);
    }
    else
    {
        std::cout << "Program " << program << " validation successful.\n";
    }
}


/******************************************************************************/
/*!
\fn     GLuint CompileShaders(char vsFilename[], char fsFilename[])
\brief
        Read the shader files, compile and link them into a program for render.
\param  const char vsFilename[]
        Vertex shader filename.
\param  const char fsFilename[]
        Fragment shader filename.
\return
        The rendering program ID.
*/
/******************************************************************************/
GLuint CompileShaders(const char vsFilename[], const char fsFilename[])
{
    const char *vsSource = ReadTextFile(vsFilename);
    const GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vsSource, NULL);
    glCompileShader(vertexShader);
    ValidateShader(vertexShader, vsFilename);           /*  Prints any errors */

    const char * fsSource = ReadTextFile(fsFilename);
    const GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fsSource, NULL);
    glCompileShader(fragmentShader);
    ValidateShader(vertexShader, fsFilename);           /*  Prints any errors */

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    ValidateProgram(program);                           /*  Print any errors */

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


/******************************************************************************/
/*!
\fn     void SetUpSkyBoxUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the skybox shader program.
\param  prog
        The given skybox shader program ID.
*/
/******************************************************************************/
void SetUpSkyBoxUniformLocations(GLuint prog)
{
    skyboxViewMatLoc = glGetUniformLocation(prog, "viewMat");
    //skyboxProjMatLoc = glGetUniformLocation(prog, "projMat"); //STUDENT ADDED
    skyboxTexCubeLoc = glGetUniformLocation(prog, "texCube");
}


/******************************************************************************/
/*!
\fn     void SetUpMainUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the main shader program.
\param  prog
        The given shader program ID.
*/
/******************************************************************************/
void SetUpMainUniformLocations(GLuint prog)
{
    mainMVMatLoc = glGetUniformLocation(prog, "mvMat");
    mainNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
    mainProjMatLoc = glGetUniformLocation(prog, "projMat");

    textureLoc = glGetUniformLocation(prog, "colorTex");

    numLightsLoc = glGetUniformLocation(prog, "numLights");
    lightPosLoc = glGetUniformLocation(prog, "lightPosVF");
    lightOnLoc = glGetUniformLocation(prog, "lightOn");

    ambientLoc = glGetUniformLocation(prog, "ambient");
    diffuseLoc = glGetUniformLocation(prog, "diffuse");
    specularLoc = glGetUniformLocation(prog, "specular");
    specularPowerLoc = glGetUniformLocation(prog, "specularPower");

    bumpTexLoc = glGetUniformLocation(prog, "bumpTex");
    normalTexLoc = glGetUniformLocation(prog, "normalTex");
    normalMappingOnLoc = glGetUniformLocation(prog, "normalMappingOn");
    parallaxMappingOnLoc = glGetUniformLocation(prog, "parallaxMappingOn");
}


/******************************************************************************/
/*!
\fn     void SetUpSphereUniformLocations(GLuint prog)
\brief
        Look up the locations of uniform variables in the sphere shader program.
\param  prog
        The given sphere shader program ID.
*/
/******************************************************************************/
void SetUpSphereUniformLocations(GLuint prog)
{
    sphereMVMatLoc = glGetUniformLocation(prog, "mvMat");
    sphereNMVMatLoc = glGetUniformLocation(prog, "nmvMat");
    sphereProjMatLoc = glGetUniformLocation(prog, "projMat");
    sphereViewMatLoc = glGetUniformLocation(prog, "viewMat");

    sphereTexCubeLoc = glGetUniformLocation(prog, "texCube");
    sphereRefLoc = glGetUniformLocation(prog, "sphereRef");
}


/******************************************************************************/
/*!
\fn     void SetUpVertexData(Mesh &mesh)
\brief
        Create VAO (graphics state), VBO (vertex data state) and 
        IBO (vertex index state) for the mesh, and feed vertex data into 
        the shaders.
\param  mesh
        The input mesh, whose VAO/VBO/IBO members will be updated and data
        will be sent to shaders.
*/
/******************************************************************************/
void SetUpVertexData(Mesh &mesh)
{
    glGenVertexArrays(1, &mesh.VAO);
    glBindVertexArray(mesh.VAO);

    glGenBuffers(1, &mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    /*  Copy vertex attributes to GPU */
    glBufferData(GL_ARRAY_BUFFER, 
                    mesh.numVertices * vertexSize, &mesh.vertexBuffer[0], 
                    GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IBO);
    /*  Copy vertex indices to GPU */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                    mesh.numIndices * indexSize, &mesh.indexBuffer[0], 
                    GL_STATIC_DRAW);

    /*  Send vertex attributes to shaders */
    for (int i = 0; i < numAttribs; ++i)
    {
        glEnableVertexAttribArray(vLayout[i].location);
        glVertexAttribPointer(  vLayout[i].location, vLayout[i].size, vLayout[i].type, 
                                vLayout[i].normalized, vertexSize, (void*)vLayout[i].offset);
    }
}


/******************************************************************************/
/*!
\fn     void UpdateLightPosViewFrame()
\brief
        Compute view-frame light positions and send them to shader when needed.
*/
/******************************************************************************/
void UpdateLightPosViewFrame()
{
    if (mainCam.moved)
    {
        for (int i = 0; i < NUM_LIGHTS; ++i)
            lightPosVF[i] = Vec3(mainCamViewMat * Vec4(lightPosWF[i], 1.0f));

        glUniform3fv(lightPosLoc, NUM_LIGHTS, ValuePtr(lightPosVF[0]));
    }
}


/******************************************************************************/
/*!
\fn     void SendLightProperties()
\brief
        Send numLights and intensities to the rendering program.
*/
/******************************************************************************/
void SendLightProperties()
{
    glUniform1i(numLightsLoc, NUM_LIGHTS);

    /*  ambient, diffuse, specular are now reflected components on the object
        surface and can be used directly as intensities in the lighting equation.
    */
    Vec4 ambient, diffuse, specular;
    ambient  = I * ambientAlbedo;
    diffuse  = I * diffuseAlbedo;
    specular = I * specularAlbedo;

    glUniform4fv(ambientLoc, 1, ValuePtr(ambient));
    glUniform4fv(diffuseLoc, 1, ValuePtr(diffuse));
    glUniform4fv(specularLoc, 1, ValuePtr(specular));

    glUniform1i(specularPowerLoc, specularPower);
}


/******************************************************************************/
/*!
\fn     ComputeObjMVMats(Mat4 MVMat[], Mat4 NMVMat[], Mat4 viewMat)
\brief
        Compute the modelview matrices for positions and normals.
\param  MVMat
        Modelview matrix for positions to be computed.
\param  NMVMat
        Modelview matrix for normals to be computed.
\param  viewMat
        Given view matrix.
*/
/******************************************************************************/
void ComputeObjMVMats(Mat4 MVMat[], Mat4 NMVMat[], Mat4 viewMat)
{
    for (int i = 0; i < ObjID::NUM_OBJS; ++i)
    {
        MVMat[i] = viewMat * obj[i].modelMat;
        NMVMat[i] = Transpose(Inverse(MVMat[i]));
    }
}


/******************************************************************************/
/*!
\fn     void ComputeMainCamMats()
\brief
        Compute the view/projection and other related matrices for user camera.
*/
/******************************************************************************/
void ComputeMainCamMats()
{
    /*  Update view transform matrix */
    if (mainCam.moved)
    {
        mainCamViewMat = mainCam.ViewMat();
        ComputeObjMVMats(mainCamMVMat, mainCamNormalMVMat, mainCamViewMat);
    }

    /*  Update projection matrix */
    if (mainCam.resized)
        mainCamProjMat = mainCam.ProjMat();
}




/******************************************************************************/
/*!
\fn     Vec3 linePlaneIntersect()
\brief
        Computes the intersection between the near plane normal and the 
        a point on the mirror.
*/
/******************************************************************************/
static Vec3 linePlaneIntersect(const Vec3& nearNormal, const Vec3& nearPoint, 
const Vec3& planePoint, const Vec3& nearToPlane) 
{
    float t = (Dot(nearNormal, nearPoint) - Dot(nearNormal, planePoint)) / Dot(nearNormal, nearToPlane);
    return (planePoint + nearToPlane * t);
}


/******************************************************************************/
/*!
\fn     void ComputeMirrorCamMats()
\brief
        Compute the view/projection and other related matrices for mirror camera.
*/
/******************************************************************************/
void ComputeMirrorCamMats()
{
    if (mainCam.moved)
    {
        /*  Computing position of user camera in mirror frame */
        Vec3 mainCamMirrorFrame = Vec3(Translate(-mirrorTranslate) * Vec4(mainCam.pos, 1.0));

        /*  If user camera is behind mirror, then mirror is not visible and no need to compute anything */
        if (mainCamMirrorFrame.z <= 0)
        {
            mirrorVisible = false;
            return;
        }
        else
            mirrorVisible = true;


        /*  In mirror frame, mirror camera position is defined as (x, y, -z) in which (x, y, z) is the
            user camera position in mirror frame.
            We also need to compute mirrorCam.pos, mirrorCam.upVec, mirrorCam.lookAt are defined in world
            frame to compute mirror cam's view matrix.
            function to compute mirrorCamViewMat
        */
        Vec3 mirrorCamMirrorFrame = Vec3(mainCamMirrorFrame.x, mainCamMirrorFrame.y, -mainCamMirrorFrame.z);

        mirrorCam.pos = Vec3(Translate(mirrorTranslate) * Vec4(mirrorCamMirrorFrame, 1.0));
        mirrorCam.upVec = BASIS[Y];
        mirrorCam.lookAt = Vec3(Translate(mirrorTranslate) * Vec4(0, 0, 0, 1));

        mirrorCamViewMat = LookAt(mirrorCam.pos, mirrorCam.lookAt, mirrorCam.upVec);

        ComputeObjMVMats(mirrorCamMVMat, mirrorCamNormalMVMat, mirrorCamViewMat);


          //@TODO Assignment5
          /*  Compute mirror camera projection matrix */
          /*  In mirror frame, the mirror camera view direction is towards the center of the mirror,
              which is the origin of this frame.

              mirrorCam.nearPlane is computed as the smallest projected length along the view direction
              of the vectors from the mirror camera to the midpoints of the mirror left, right, bottom
              and top edges. These midpoints are pre-defined in mirror frame as (0.5, 0, 0) [left],
              (-0.5, 0, 0) [right], (0, -0.5, 0) [bottom] and (0, 0.5, 0) [top].

              mirrorCam.farPlane is set to INFINITY.

              leftPlane, rightPlane, bottomPlane, topPlane are computed based on the intersections between
              the near plane and the vectors from camera to the left, right, bottom and top edge midpoints.
              After you've done with this computation, you would see the reflected image with a quite narrow
              view angle (which is physically correct). However, in order to widen the view angle, we would
              multiply all leftPlane, rightPlane, bottomPlane, topPlane by 2.
          */
          /*  ... */
          /*  Compute mirror cam's projection matrix */
        Vec3 mirror_l = Vec3(0.5, 0, 0);
        Vec3 mirror_r = Vec3(-0.5, 0, 0);
        
        Vec3 mirror_b = Vec3(0, -0.5, 0);
        Vec3 mirror_t = Vec3(0, 0.5, 0);

        Vec3 mirror_viewNormal = Normalize(-mirrorCamMirrorFrame);

        Vec3 mEyeP(mirrorCamMirrorFrame); 

        Vec3 leftLineDir = Normalize(mirror_l - mEyeP);
        Vec3 rightLineDir = Normalize(mirror_r - mEyeP);
        Vec3 bottomLineDir = Normalize(mirror_b - mEyeP);
        Vec3 topLineDir = Normalize(mirror_t - mEyeP);


        float dis_left = Dot(mirror_viewNormal, leftLineDir);
        float dis_right = Dot(mirror_viewNormal, rightLineDir);

        float dis_bottom = Dot(mirror_viewNormal, bottomLineDir);
        float dis_top = Dot(mirror_viewNormal, topLineDir);

        mirrorCam.nearPlane = std::min(std::min(dis_left, dis_right), std::min(dis_bottom, dis_top));


        Vec3 nearPoint = mirror_viewNormal * mirrorCam.nearPlane + (mEyeP);

        Vec3 nToM_l = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_l, leftLineDir);
        Vec3 nToM_r = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_r, rightLineDir);
        Vec3 nToM_b = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_b, bottomLineDir);
        Vec3 nToM_t = linePlaneIntersect(mirror_viewNormal, nearPoint, mirror_t, topLineDir);

        mirrorCam.leftPlane = -2.f * Distance(nearPoint, nToM_l);
        mirrorCam.rightPlane = 2.f * Distance(nearPoint, nToM_r);
        mirrorCam.bottomPlane = -2.f * Distance(nearPoint, nToM_b);
        mirrorCam.topPlane = 2.f * Distance(nearPoint, nToM_t);

        mirrorCamProjMat = Frustum(mirrorCam.leftPlane, mirrorCam.rightPlane, mirrorCam.bottomPlane, 
        mirrorCam.topPlane, mirrorCam.nearPlane, mirrorCam.farPlane);
    }
}


/******************************************************************************/
/*!
\fn     void ComputeSphereCamMats()
\brief
        Compute the view/projection and other related matrices for sphere camera.
*/
/******************************************************************************/
void ComputeSphereCamMats()
{
    //@TODO Assignment5
    /*  Compute the lookAt positions for the 6 faces of the sphere cubemap. 
        The sphere camera is at spherePos.
        The front and back faces are -z and +z.
        The left and right faces are -x and +x.
        The bottom and top faces are -y and +y.
    */
    Vec3 lookAt[CubeFaceID::NUM_FACES];
    /*  ... */

    //IDEA: store the lookAts and upVecs  in order of the textures...
    // 
    //+x,-x
    lookAt[0] = spherePos + Vec3(1, 0, 0);
    lookAt[1] = spherePos + Vec3(-1, 0, 0);
    
    //+y,-y
    lookAt[2] = spherePos + Vec3(0, 1, 0);
    lookAt[3] = spherePos + Vec3(0, -1, 0);

    //+z,-z
    lookAt[4] = spherePos + Vec3(0, 0, 1);
    lookAt[5] = spherePos + Vec3(0, 0, -1);


    //@TODO Assignment5
    /*  Compute upVec for the 6 faces of the sphere cubemap.
        The front, back, left and right faces are flipped upside-down.
        The top face's upVec is pointing forward and the bottom face's
        upVec is pointing backward.
    */
    Vec3 upVec[CubeFaceID::NUM_FACES];
    /*  ... */


    upVec[0] = Vec3(0, -1, 0);
    upVec[1] = Vec3(0, -1, 0);

    upVec[2] = Vec3(0, 0, 1);
    upVec[3] = Vec3(0, 0, -1);

    upVec[4] = Vec3(0, -1, 0);
    upVec[5] = Vec3(0, -1, 0);


    //upVec[2] = Vec3(0, 1, 0);    // +y - Pointing upward along positive y-axis
    //upVec[3] = Vec3(0, -1, 0);   // -y - Pointing downward along negative y-axis

    for (int f = 0; f < CubeFaceID::NUM_FACES; ++f)
    {
        sphereCamViewMat[f] = LookAt(spherePos, lookAt[f], upVec[f]);
        ComputeObjMVMats(sphereCamMVMat[f], sphereCamNormalMVMat[f], sphereCamViewMat[f]);
    }


    //@TODO Assignment5
    /*  Use Perspective function to ompute the projection matrix sphereCamProjMat so that 
        from the camera position at the cube center, we see a complete face of the cube.
        The near plane distance is 0.01f. The far plane distance is equal to mainCam's farPlane.
    */
    /*  ... */
    sphereCamProjMat = Perspective(HALF_PI, 1.f, .01f, mainCam.farPlane);
}


/******************************************************************************/
/*!
\fn     void SendMVMat(const Object &obj)
\brief
        Send object's model-view matrices to the shader.
\param  obj
        The object whose model-view matrices we want to send.
*/
/******************************************************************************/
void SendMVMat(const Mat4 &mvMat, const Mat4 &nmvMat, GLint mvMatLoc, GLint nmvMatLoc)
{
    /*  Send transformation matrices to shaders */
    glUniformMatrix4fv(mvMatLoc, 1, GL_FALSE, ValuePtr(mvMat));
    glUniformMatrix4fv(nmvMatLoc, 1, GL_FALSE, ValuePtr(nmvMat));
}


/******************************************************************************/
/*!
\fn     void SendViewMat(Mat4 viewMat, GLint viewMatLoc)
\brief
        Send view matrix to the corresponding variable.
        We send this separately because the skybox rendering only needs 
        view matrix but not modeling or projection matrix.
\param  viewMat
        The view matrix to be sent.
\param  viewMatLoc
        The location of the variable to send to.
*/
/******************************************************************************/
void SendViewMat(Mat4 viewMat, GLint viewMatLoc)
{
    glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, ValuePtr(viewMat));
}


/******************************************************************************/
/*!
\fn     void SendProjMat()
\brief
        Send projection matrix to the shader.
\param  obj
        The object whose projection matrix we want to send.
*/
/******************************************************************************/
void SendProjMat(Mat4 projMat, GLint projMatLoc)
{
    glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, ValuePtr(projMat));
}


/******************************************************************************/
/*!
\fn     void SetUpObjTextures()
\brief
        Read texture images from files, then copy them to graphics memory.
        These textures will be combined with light colors for the objects 
        in the scene.
*/
/******************************************************************************/
void SetUpObjTextures()
{
    glGenTextures(ImageID::NUM_IMAGES, texID);

    unsigned char *imgData;
    int imgWidth, imgHeight, numComponents;

    /*  Mirror and sphere will not use existing textures so we'll set them up separately */
    for (int i = 0; i < ImageID::NUM_IMAGES; ++i)
        if (i != ImageID::MIRROR_TEX && i != ImageID::SPHERE_TEX)
        {
            if (ReadImageFile(objTexFile[i], &imgData, &imgWidth, &imgHeight, &numComponents) == 0)
            {
                std::cerr << "Reading " << objTexFile[i] << " failed.\n";
                exit(1);
            }

            /*  Bind corresponding texture ID */
            glBindTexture(GL_TEXTURE_2D, texID[i]);

            /*  Copy image data to graphics memory */
            if (numComponents == 3)
                glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGB8, imgWidth, imgHeight, 0, 
                                GL_RGB, GL_UNSIGNED_BYTE, imgData);
            else
                glTexImage2D(   GL_TEXTURE_2D, 0, GL_RGBA8, imgWidth, imgHeight, 0, 
                                GL_RGBA, GL_UNSIGNED_BYTE, imgData);

            /*  Done with raw image data so delete it */
            free(imgData);

            /*  Generate texture mipmaps. */
            glGenerateMipmap(GL_TEXTURE_2D);

            /*  Set up texture behaviors */
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
}


/******************************************************************************/
/*!
\fn     void Bump2Normal(   const unsigned char *bumpImg, 
                            unsigned char *normalImg, 
                            int width, int height)
\brief
        Compute normal map from bump map.
\param  bumpImg
        Given 1D bump map.
\param  normalImg
        3D normal map to be computed.
\param  width
        Width of the texture map.
\param  height
        Height of the texture map.
*/
/******************************************************************************/
void Bump2Normal(const unsigned char *bumpImg, unsigned char *normalImg, int width, int height)
{
    /*  Compute the normal map from bump map using the formula in the lecture note, in which a is 40.
        For the leftmost/rightmost columns or bottom/top rows, the formula should use the 
        two leftmost/rightmost columns or two bottom/top rows and 0.5*a.
        Note that the formula in the lecture note assumes the bump/height values are within [0, 1].
        But our bumpImg values range from 0 to 255.
    */
    float a = 40.0f;

    float alpha_delta_bt = a;
    float alpha_delta_lr = a;

    for (int i = 0; i < height; i++) {

        for (int j = 0; j < width; j++) {

            int idx = i * width + j;
            //leftmost? or just a left?
            float Bi_minus_1;
            if (j == 0)
            { 
                alpha_delta_lr = a * .5f;
                Bi_minus_1 = static_cast<float>(bumpImg[idx]) / 255.f;
            }
            else 
            { 
                Bi_minus_1 = static_cast<float>(bumpImg[idx - 1]) / 255.f;
            }

            //rightmost? or just right?
            float Bi_plus_1;
            if (j == width - 1) 
            {
                alpha_delta_lr = a * .5f;
                Bi_plus_1 = static_cast<float>(bumpImg[idx]) / 255.f ;
            }
            else 
            { 
                Bi_plus_1 = static_cast<float>(bumpImg[idx + 1]) / 255.f ;
            }


            //top most? or just a top?
            float Bj_plus_1;
            if (i == height-1) 
            { 
                alpha_delta_bt = a * .5f;
                Bj_plus_1 = static_cast<float>(bumpImg[idx]) / 255.f;
            }
            else 
            { 
                //if ((idx + width) >= width * width) { std::cout << "uh\n"; }
                Bj_plus_1 = static_cast<float>(bumpImg[idx + width]) / 255.f ;
            }

            float Bj_minus_1;
            if (i == 0) 
            {
                alpha_delta_bt = a * .5f;
                Bj_minus_1 = static_cast<float>(bumpImg[idx]) / 255.f ;
            }
            else 
            { 
            /*    if ((idx - width) < 0) { std::cout << "uh\n"; }*/
                Bj_minus_1 = static_cast<float>(bumpImg[idx - width]) / 255.f ;
            } //bottom most? or just bottom.

            float Bu_z = (Bi_plus_1 - Bi_minus_1) * alpha_delta_lr;
            float Bv_z = (Bj_plus_1 - Bj_minus_1) * alpha_delta_bt;

    
            Vec3 Bu = Vec3(1, 0, Bu_z);
            Vec3 Bv = Vec3(0, 1, Bv_z);
        
            Vec3 pert_normal = glm::normalize(glm::cross(Bu,Bv));

        
            normalImg[idx * 3] = static_cast<unsigned char>((pert_normal.y  + 1.0f) * 127.5f); //r
            normalImg[idx * 3 + 1] = static_cast<unsigned char>((pert_normal.x + 1.0f) * 127.5f);//g
            normalImg[idx * 3 + 2] = static_cast<unsigned char>((pert_normal.z  + 1.0f) * 127.5f);//b

            alpha_delta_bt = a;
            alpha_delta_lr = a;
        }
    }


  
   

}


/******************************************************************************/
/*!
\fn     void SetUpBaseBumpNormalTextures()
\brief
        Set up the bump map and normal map for normal mapping and parallax mapping.
*/
/******************************************************************************/
void SetUpBaseBumpNormalTextures()
{
    unsigned char *bumpImgData, *normalImgData;
    int imgWidth, imgHeight, numComponents;

    /*  Load bump image */
    if (ReadImageFile(bumpTexFile, &bumpImgData, &imgWidth, &imgHeight, &numComponents) == 0)
    {
        std::cerr << "Reading " << bumpTexFile << " failed.\n";
        exit(1);
    }

    /*  Create normal image */
    normalImgData = (unsigned char *)malloc(imgWidth * imgHeight * 3 * sizeof(unsigned char));

    Bump2Normal(bumpImgData, normalImgData, imgWidth, imgHeight);

    /*  Generate texture ID for bump image and copy it to GPU */
    /*  Bump image will be used to compute the offset in parallax mapping */
    glGenTextures(1, &bumpTexID);
    glBindTexture(GL_TEXTURE_2D, bumpTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, imgWidth, imgHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bumpImgData);

    free(bumpImgData);

    /*  Generate texture mipmaps. */
    glGenerateMipmap(GL_TEXTURE_2D);
    /*  Set up texture behaviors */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


    /*  Generate texture ID for normal image and copy it to GPU */
    glGenTextures(1, &normalTexID);
    glBindTexture(GL_TEXTURE_2D, normalTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, imgWidth, imgHeight, 0,
        GL_RGB, GL_UNSIGNED_BYTE, normalImgData);

    free(normalImgData);

    /*  Generate texture mipmaps. */
    glGenerateMipmap(GL_TEXTURE_2D);

    /*  Set up texture behaviors */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}


/******************************************************************************/
/*!
\fn     void CopySubTexture(    unsigned char *destTex, const unsigned char *srcTex,
                                int size, int imgWidth,
                                int verticalOffset, int horizontalOffset, 
                                bool verticalFlip, bool horizontalFlip,
                                int numComponents)
\brief
        Copy from an area from srcTex to destTex.
        The destTex has dimensions size*size.
        The columns in srcTex will be from verticalOffset to verticalOffset + size - 1.
        The rows in srcTex will be from horizontalOffset to horizontalOffset + size - 1.
        If verticalFlip/horizontalFlip is true then we have to flip the columns/rows within
        the copied range vertically/horizontally.
\param  size
        Dimension of the copied area.
\param  imgWidth
        Width of srcTex.
\param  verticalOffset
        First column of the copied area in srcTex.
\param  horizontalOffset
        First row of the copied area in srcTex.
\param  verticalFlip
        Whether we need to flip the columns within the copied area.
\param  horizontalFlip
        Whether we need to flip the rows within the copied area.
*/
/******************************************************************************/
void CopySubTexture(    unsigned char *destTex, const unsigned char *srcTex,
                        int size, int imgWidth,
                        int verticalOffset, int horizontalOffset, 
                        bool verticalFlip, bool horizontalFlip,
                        int numComponents)
{

    /*  Copy from srcTex to destTex and flip if needed */
    // we are treating a specific AREA in srcTex...
    int iter_y = 0;
    for (int y = verticalOffset; y < verticalOffset + size; ++y, ++iter_y)
    {
        int src_y = verticalFlip ? (verticalOffset + size - iter_y - 1) * imgWidth : y * imgWidth;
        int dest_y = ((y - verticalOffset) * size);
        
        int iter_x = 0;
        for (int x = horizontalOffset; x < horizontalOffset + size; ++x, ++iter_x)
        {
            int src_indx = horizontalFlip ? numComponents * (src_y + (horizontalOffset + size- iter_x -1)) : numComponents * (src_y + x);
            int dest_indx =  numComponents * (dest_y + (x - horizontalOffset));

            //copy the components of the texel.
            for (int i = 0; i < numComponents; ++i)
            {
                destTex[dest_indx + i] = srcTex[src_indx + i]; //r,g,b,a?
            }
        }

    }
}


/******************************************************************************/
/*!
\fn     void SetUpSkyBoxTexture()
\brief
        Set up the cubemap texture from the skybox image.
*/
/******************************************************************************/
void SetUpSkyBoxTexture()
{
    unsigned char *cubeImgData, *cubeFace[CubeFaceID::NUM_FACES];
    int imgWidth, imgHeight, numComponents;

    if (ReadImageFile(skyboxTexFile, &cubeImgData, &imgWidth, &imgHeight, &numComponents) == 0)
    {
        std::cerr << "Reading " << skyboxTexFile << " failed.\n";
        exit(1);
    }

    skyboxFaceSize = imgHeight / 3;

    int imgSizeBytes = sizeof(unsigned char) * skyboxFaceSize * skyboxFaceSize * numComponents;

    for (int f = 0; f < CubeFaceID::NUM_FACES; ++f)
        cubeFace[f] = (unsigned char *)malloc(imgSizeBytes);
    
    /*  Copy the texture from the skybox image to 6 textures using CopySubTexture */ 
    /*  imgWidth is the width of the original image, while skyboxFaceSize is the size of each face */
    /*  The cubemap layout is as described in the assignment specs */
 
    int vert_offset = 0;
    int horz_offset = 0;
    bool horz_flip = false;
    bool vert_flip = false;
    for (int f = 0; f < CubeFaceID::NUM_FACES; ++f) 
    {   
        //Horizontal offsets
        if (f == 0 || f == 5) 
        {
            horz_offset = skyboxFaceSize;
        }
        else 
        {
            vert_flip = true;
            horz_flip = true;
            horz_offset = skyboxFaceSize * ((f - 1) % 4);
        }

        //Vertical Offsets
        if (f == 0) 
        {
            vert_offset = 0;
        }
        else if (f == 5) 
        {

            vert_offset = skyboxFaceSize * 2;
        }
        else 
        {
            vert_offset = skyboxFaceSize;
        }


        CopySubTexture(cubeFace[f], cubeImgData, skyboxFaceSize, 
                       imgWidth, vert_offset, horz_offset, 
                       vert_flip, horz_flip, numComponents);

        horz_flip = false;
        vert_flip = false;

    }


    glGenTextures(1, &skyboxTexID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexID);


    /*  Copy the 6 textures to the GPU cubemap texture object, and set appropriate texture parameters */
    GLuint internalFormat, format;
    if (numComponents == 3)
    {
        internalFormat = GL_RGB8;
        format = GL_RGB;
    }
    else
    {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
    }
    /*  ... */

    //Right
    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        0, internalFormat, skyboxFaceSize, skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[3]
    );

    //Left
    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        0, internalFormat, skyboxFaceSize, skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[1]
    );

    //Top
    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        0, internalFormat, skyboxFaceSize, skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[5]
    );


    //Bottom
    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        0, internalFormat, skyboxFaceSize, skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[0]
    );

    //Back
    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        0, internalFormat, skyboxFaceSize, skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[4]
    );

    //Front
    glTexImage2D(
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        0, internalFormat, skyboxFaceSize, skyboxFaceSize, 0, format, GL_UNSIGNED_BYTE, cubeFace[2]
    );

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    free(cubeImgData);
    for (int f = 0; f < CubeFaceID::NUM_FACES; ++f)
        free(cubeFace[f]);
}


/******************************************************************************/
/*!
\fn     void SetUpMirrorTexture()
\brief
        Set up texture and frame buffer objects for rendering mirror reflection.
*/
/******************************************************************************/
void SetUpMirrorTexture()
{
    glGenTextures(1, &mirrorTexID);
    glBindTexture(GL_TEXTURE_2D, mirrorTexID);

	/*  Some graphics drivers don't support glTexStorage2D */
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, mirrorCam.width, mirrorCam.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mirrorCam.width, mirrorCam.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    /*  Calling glTexImage2D with levels = 0 will only generate one level of texture,
        so let's not use mipmaps here.
        The visual difference in this case is hardly recognizable.
    */
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glGenFramebuffers(1, &mirrorFrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, mirrorFrameBufferID);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mirrorTexID, 0);
}


/******************************************************************************/
/*!
\fn     void SetUpSphereTexture(unsigned char *sphereCubeMapData[])
\brief
        Set up texture object for rendering sphere reflection/refraction.
*/
/******************************************************************************/
void SetUpSphereTexture(unsigned char *sphereCubeMapData[])
{
    glGenTextures(1, &sphereTexID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sphereTexID);

    for (int f = 0; f < CubeFaceID::NUM_FACES; ++f)
        glTexImage2D(   GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, GL_RGBA8, skyboxFaceSize, skyboxFaceSize,
                        0, GL_RGBA, GL_UNSIGNED_BYTE, sphereCubeMapData[f]);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}


/******************************************************************************/
/*!
\fn     void SendCubeTexID(int texID, GLint texCubeLoc)
\brief
        Send cubemap texture ID to the corresponding variable.
\param  texID
        - ID to be sent.
\param  texCubeLoc
        Location of the variable to send to.
*/
/******************************************************************************/
void SendCubeTexID(int texID, GLint texCubeLoc)
{     
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
    glUniform1i(texCubeLoc, 0);
}


/******************************************************************************/
/*!
\fn     void SendMirrorTexID()
\brief
        Send mirror texture ID to the corresponding variable.
*/
/******************************************************************************/
void SendMirrorTexID()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mirrorTexID);
    glUniform1i(textureLoc, 0);
}


/******************************************************************************/
/*!
\fn     void SendObjTexID(GLuint texID, int activeTex)
\brief
        Send object's texture ID to the shader.
\param  texID
        The texture object we want to bind to.
\param  activeTex
        The active texture handler we want to use to map with the sampler variable.
\param  texLoc
        The location of the uniform sampler in the shader.
*/
/******************************************************************************/
void SendObjTexID(GLuint texID, int activeTex, GLint texLoc)
{
    glActiveTexture(GL_TEXTURE0 + activeTex);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(texLoc, activeTex);
}




/******************************************************************************/
/*!
\fn     void SetUp()
\brief
        Set up the render program and graphics-related data for rendering.
*/
/******************************************************************************/
void SetUp()
{
    /*  Initialization for fps estimation */
    currTime = clock();
    frameCount = 0;
    secCount = 0;

    /*  Set Up Scene:
        - Obj attributes, which include mesh type, size, pos, color/texture ...
        - Light positions
    */
    SetUpScene();

    for (int i = 0; i < ProgType::NUM_PROGTYPES; ++i)
        prog[i] = CompileShaders(file[i][ShaderType::VERTEX_SHADER], file[i][ShaderType::FRAGMENT_SHADER]);


    /*  Send mesh data only */
    for (int i = 0; i < MeshID::NUM_MESHES; ++i)
        SetUpVertexData(mesh[i]);


    /*  Set up textures for objects in the scene */
    SetUpObjTextures();

    /*  Set up bump map and normal map for the base object */
    SetUpBaseBumpNormalTextures();

    /*  Set up skybox texture for background rendering */
    SetUpSkyBoxTexture();

    /*  Set up texture object for mirror reflection. This texture object hasn't stored any data yet. 
        We will render the reflected data for this texture on the fly.
    */
    SetUpMirrorTexture();


    /*  Look up for the locations of the uniform variables in the shader programs */
    glUseProgram(prog[ProgType::SKYBOX_PROG]);
    SetUpSkyBoxUniformLocations(prog[ProgType::SKYBOX_PROG]);

    glUseProgram(prog[ProgType::SPHERE_PROG]);
    SetUpSphereUniformLocations(prog[ProgType::SPHERE_PROG]);

    glUseProgram(prog[ProgType::MAIN_PROG]);
    SetUpMainUniformLocations(prog[ProgType::MAIN_PROG]);


    /*  Send light info to the programs.
        Vertex data and textures are copied to the graphics memory, which are mapped to the 
        corresponding variables in shaders when needed. Hence, no program needs to be specified yet.
        However, uniform light data is sent directly to shaders, hence we have to indicate the
        programs to be sent to.
    */
    SendLightProperties();


    /*  Drawing using filled mode */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    /*  Hidden surface removal */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /*  Enable writing to depth buffer */
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);     /*  For efficiency, not drawing back-face */
    glCullFace(GL_BACK);
}


/******************************************************************************/
/*!
\fn     void CleanUp()
\brief
        Clean up the graphics-related stuffs before shutting down.
*/
/******************************************************************************/
void CleanUp()
{
    TwTerminate();      /*  Terminate AntTweakBar */

    glBindVertexArray(0);
    
    for (int i = 0; i < MeshID::NUM_MESHES; ++i)
    {
        glDeleteVertexArrays(1, &mesh[i].VAO);
        glDeleteBuffers(1, &mesh[i].VBO);
        glDeleteBuffers(1, &mesh[i].IBO);
    }

    glDeleteTextures(ImageID::NUM_IMAGES, texID);
    glDeleteTextures(1, &bumpTexID);
    glDeleteTextures(1, &normalTexID);
    glDeleteTextures(1, &skyboxTexID);
    glDeleteTextures(1, &mirrorTexID);
    glDeleteTextures(1, &sphereTexID);

    glDeleteFramebuffers(1, &mirrorFrameBufferID);
    
    //glDeleteProgram(renderProg);
    //for (int i = 0; i < ProgType::NUM_PROGTYPES; ++i)
    for (int i = 0; i < 2; ++i)
        glDeleteProgram(prog[i]);

    exit(0);    /*  successful run */
}


/******************************************************************************/
/*!
\fn     void Resize(int w, int h)
\brief
        Update viewport and projection matrix upon window resize.
\param  w
        The new width of the window
\param  h
        The new height of the window
*/
/******************************************************************************/
void Resize(int w, int h)
{
    mainCam.width = w;
    mainCam.height = h;

    TwWindowSize(mainCam.width, mainCam.height);    /*  Send AntTweakBar updated window size */

    glutPostRedisplay();    /*  Set flag to force re-rendering */
}


/******************************************************************************/
/*!
\fn     void EstimateFPS()
\brief
        Estimating FPS. This only updates the FPS about once per second.
*/
/******************************************************************************/
void EstimateFPS()
{
    ++frameCount;

    prevTime = currTime;
    currTime = clock();
    secCount += 1.0f * (currTime - prevTime) / CLOCKS_PER_SEC;

    if (secCount > 1.0f)
    {
        fps = frameCount / secCount;

        frameCount = 0;
        secCount = 0;
    }
}


/******************************************************************************/
/*!
\fn     void RenderSkybox(const Mat4 &viewMat)
\brief
        Render the background using skybox cubemap.
\param  viewMat
        The view transform matrix to define the orientation of our camera.
*/
/******************************************************************************/
void RenderSkybox(const Mat4 &viewMat)
{
    glClearBufferfv(GL_DEPTH, 0, &one);
  /*  glDepthMask(GL_FALSE);*/
    glUseProgram(prog[ProgType::SKYBOX_PROG]);

    SendCubeTexID(skyboxTexID, skyboxTexCubeLoc);
    SendViewMat(viewMat, skyboxViewMatLoc);
    //SendProjMat(projMat, skyboxProjMatLoc); //STUDENT ADDED

    /*  Just trigger the skybox shaders, which hard-code the full-screen quad drawing */
    /*  No vertices are actually sent */
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
   /* glDepthMask(GL_TRUE);*/
}


/******************************************************************************/
/*!
\fn     void RenderObj(const Object &obj)
\brief
        Render an object.
\param  obj
        The object that we want to render.
*/
/******************************************************************************/
void RenderObj(const Object &obj)
{
    /*  Tell shader to use obj's VAO for rendering */
    glBindVertexArray(mesh[obj.meshID].VAO);
    glDrawElements(GL_TRIANGLES, mesh[obj.meshID].numIndices, GL_UNSIGNED_INT, nullptr);
}


/******************************************************************************/
/*!
\fn     void RenderSphere()
\brief
        Render the sphere using its own shader program.
*/
/******************************************************************************/
void RenderSphere()
{
    glUseProgram(prog[ProgType::SPHERE_PROG]);

    SendCubeTexID(sphereTexID, sphereTexCubeLoc);

    /*  Indicate whether we want reflection/refraction or both */
    glUniform1i(sphereRefLoc, sphereRef);

    /*  We need view mat to know our camera orientation */
    SendViewMat(mainCamViewMat, sphereViewMatLoc);

    /*  These are for transforming vertices on the sphere */
    SendMVMat(mainCamMVMat[ObjID::SPHERE], mainCamNormalMVMat[ObjID::SPHERE], sphereMVMatLoc, sphereNMVMatLoc);
    SendProjMat(mainCamProjMat, sphereProjMatLoc);

    RenderObj(obj[ObjID::SPHERE]);
}


/******************************************************************************/
/*!
\fn     void RenderObjsBg(  Mat4 MVMat[], Mat4 normalMVMat[], 
                            Mat4 viewMat, Mat4 projMat, 
                            int viewportWidth, int viewportHeight,
                            int renderPass)
\brief
        Render background and objects in the scene, except the sphere.
\param  MVMat
        The mode-view matrices of the objects.
\param  normalMVMat
        The normal mode-view matrices of the normals on the objects.
\param  viewMat
        The view transform matrix of the camera to render the skybox background.
\param  projMat
        The projection matrix of the camera.
\param  viewportWidth
        The width of the viewport to render to.
\param  viewportHeight
        The height of the viewport to render to.
\param  renderPass
        Whether we are rendering textures for sphere reflection/refraction, or
        mirror reflection, or rendering objects onto the screen.
        We need this flag because each pass only render certain objects.
*/
/******************************************************************************/
void RenderObjsBg(  Mat4 MVMat[], Mat4 normalMVMat[], Mat4 viewMat, Mat4 projMat, 
                    int viewportWidth, int viewportHeight,
                    int renderPass)
{
    /*  We need to set this here because the onscreen rendering will use a bigger viewport than
        the rendering of sphere/mirror reflection/refraction texture
    */
    glViewport(0, 0, viewportWidth, viewportHeight);
    

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderSkybox(viewMat); //student added projMat


    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* @TODO Assignment 5: Remove this line  */
    glUseProgram(prog[ProgType::MAIN_PROG]);
    UpdateLightPosViewFrame();
    SendProjMat(projMat, mainProjMatLoc);

    /*  Send object texture and render them */
    for (int i = 0; i < ObjID::NUM_OBJS; ++i)
        if (i == ObjID::SPHERE)
            continue;           /*  Will use sphere rendering program to apply reflection & refraction textures on sphere */
        else

        if (renderPass == RenderPass::MIRRORTEX_GENERATION 
            && (i == ObjID::MIRROR || i == ObjID::MIRRORBASE1 || i == ObjID::MIRRORBASE2 || i == ObjID::MIRRORBASE3))
            continue;           /*  Not drawing objects behind mirror & mirror itself */
        else

        if (renderPass == RenderPass::SPHERETEX_GENERATION && (i == ObjID::MIRROR))
            continue;           /*  Not drawing mirror when generating reflection/refraction texture for sphere to avoid inter-reflection */
        else
        {
            if (i == ObjID::MIRROR)
            {
                SendMirrorTexID();
                glUniform1i(lightOnLoc, 0);     /*  disable lighting on mirror surface */
            }
            else
            {
                SendObjTexID(texID[obj[i].imageID], ActiveTexID::COLOR, textureLoc);
                glUniform1i(lightOnLoc, 1);     /*  enable lighting for other objects */
            }

            SendMVMat(MVMat[i], normalMVMat[i], mainMVMatLoc, mainNMVMatLoc);

            if (i == ObjID::BASE)   /*  apply normal mapping / parallax mapping for the base */
            {
                SendObjTexID(normalTexID, ActiveTexID::NORMAL, normalTexLoc);
                glUniform1i(normalMappingOnLoc, true);
                glUniform1i(parallaxMappingOnLoc, parallaxMappingOn);

                if (parallaxMappingOn)
                    SendObjTexID(bumpTexID, ActiveTexID::BUMP, bumpTexLoc);
            }
            else                       /*  not apply normal mapping / parallax mapping for other objects */
            {
                glUniform1i(normalMappingOnLoc, false);
                glUniform1i(parallaxMappingOnLoc, false);
            }

            /*  The mirror surface is rendered to face away to simulate the flipped effect.
                Hence we need to perform front-face culling for it.
                Other objects use back-face culling as usual.
            */
            if (i == ObjID::MIRROR)
                glCullFace(GL_FRONT);

            RenderObj(obj[i]);

            /*  Trigger back-face culling again */
            if (i == ObjID::MIRROR)
                glCullFace(GL_BACK);
        }
}


/******************************************************************************/
/*!
\fn     void RenderToSphereCubeMapTexture(unsigned char *sphereCubeMapTexture[])
\brief
        Render the scene to 6 faces of the cubemap for sphere 
        reflection/refraction later.
\param  sphereCubeMapTexture
        Buffers to store the 6 faces of the cubemap texture.
*/
/******************************************************************************/
void RenderToSphereCubeMapTexture(unsigned char *sphereCubeMapTexture[])
{
    /*  Theoretically the rendering to cubemap texture can be done in the same way as 2D texture: 
        rendering straight to the GPU cubemap texture object, similar to what we do for the 
        2D mirror texture in RenderToMirrorTexture.
        However, some graphics drivers don't seem to implement the framebuffer cubemap texture properly.
        So we do the cubemap texture writing manually here: copy the framebuffer to CPU texture data, 
        then copy that data to GPU texture object later (in SetUpSphereTexture function).
    */

    GLuint sphereFrameBufferID;
    glGenFramebuffers(1, &sphereFrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, sphereFrameBufferID);

    GLuint sphereFrameBufferTexID;
    glGenTextures(1, &sphereFrameBufferTexID);
    glBindTexture(GL_TEXTURE_2D, sphereFrameBufferTexID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, skyboxFaceSize, skyboxFaceSize,
        0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    for (int i = 0; i < CubeFaceID::NUM_FACES; ++i)
    {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, sphereFrameBufferTexID, 0);

        RenderObjsBg(   sphereCamMVMat[i], sphereCamNormalMVMat[i], sphereCamViewMat[i], sphereCamProjMat,
                        skyboxFaceSize, skyboxFaceSize,
                        RenderPass::SPHERETEX_GENERATION);

        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, skyboxFaceSize, skyboxFaceSize, GL_RGBA, GL_UNSIGNED_BYTE, sphereCubeMapTexture[i]);
    }

    glDeleteTextures(1, &sphereFrameBufferTexID);
    glDeleteFramebuffers(1, &sphereFrameBufferID);
}


/******************************************************************************/
/*!
\fn     void RenderToMirrorTexture()
\brief
        Render the scene to the texture for mirror reflection. This texture was 
        already bound to mirrorFrameBufferID in SetUpMirrorTexture function.
*/
/******************************************************************************/
void RenderToMirrorTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mirrorFrameBufferID);
    RenderObjsBg(   mirrorCamMVMat, mirrorCamNormalMVMat, mirrorCamViewMat, mirrorCamProjMat,
                    mirrorCam.width, mirrorCam.height,
                    RenderPass::MIRRORTEX_GENERATION);
}


/******************************************************************************/
/*!
\fn     void RenderToScreen()
\brief
        Render the scene to the default framebuffer.
*/
/******************************************************************************/
void RenderToScreen()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    RenderObjsBg(   mainCamMVMat, mainCamNormalMVMat, mainCamViewMat, mainCamProjMat,
                    mainCam.width, mainCam.height,
                    RenderPass::NORMAL);
}




bool firstFrame = true;

/******************************************************************************/
/*!
\fn     void Render()
\brief
        Render function for update & drawing.
        This function will perform forward shading if deferredShading == false.
        Otherwise it will perform geometry pass followed by light pass of 
        deferred shading.
*/
/******************************************************************************/
void Render()
{
    ComputeMainCamMats();
    ComputeMirrorCamMats();

    /*  The texture used for sphere reflection/refraction is view-independent, 
        so it only needs to be rendered once in the beginning
    */
    if (firstFrame)
    {
        ComputeSphereCamMats();

        unsigned char *sphereCubeMapData[CubeFaceID::NUM_FACES];
        for (int f = 0; f < CubeFaceID::NUM_FACES; ++f)
            sphereCubeMapData[f] = (unsigned char *)malloc(skyboxFaceSize*skyboxFaceSize * 4 * sizeof(unsigned char));

        /*  Theoretically the rendering to cubemap texture can be done in the same way as 2D texture: 
            rendering straight to the GPU texture object, similar to what we do for the 
            2D mirror texture below.
            However, some graphics drivers don't seem to implement the framebuffer cubemap texture properly.
            So we do the cubemap texture generation manually here: copy the framebuffer to CPU texture data, 
            then copy that data to the GPU texture object.
        */
        RenderToSphereCubeMapTexture(sphereCubeMapData);
        SetUpSphereTexture(sphereCubeMapData);

        for (int f = 0; f < CubeFaceID::NUM_FACES; ++f)
            free(sphereCubeMapData[f]);

        firstFrame = false;
    }


    /*  The texture for planar reflection is view-dependent, so it needs to be rendered on the fly,
        whenever the mirror is visible and camera is moving 
    */
    if (mirrorVisible && mainCam.moved)
        RenderToMirrorTexture();

    /*  Render the scene, except the sphere to the screen */
    RenderToScreen();

    /*  This is done separately, as it uses a different shader program for reflection/refraction */
    RenderSphere();


    /*  Reset */
    mainCam.moved = false;
    mainCam.resized = false;

    EstimateFPS();
    TwDraw();   /*  Update AntTweakBar */

    glutSwapBuffers();
}

/*  YAY ! END OF THE SUPER TEDIOUS DOCUMENTATION !!! */