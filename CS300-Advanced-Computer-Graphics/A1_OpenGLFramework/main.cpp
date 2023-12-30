/*****************************************************************************/
/*!
\file main.cpp
\author Yap Jin Ying Akina and Caleb Kissinger
\par email: caleb.kissinger\@digipen.edu
\par DigiPen login: caleb.kissinger
\par Course: A1
\par Assignment 1
\date 19 SEPTEMBER 2023

Copyright (C) 2023 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/*****************************************************************************/

#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

static void Init();
static bool LoadModel(const std::string& path); //return value is not used.
static void Draw();
static void Update();
static void Cleanup();
static bool CreateLogFile(std::string file_name);
static void ErrorCallbackForGLFW(int error, char const* description);
template <typename... Args> bool WriteToFile(std::string file_name, Args&&... args);
static void WindowResizeCallbackForGLFW(GLFWwindow* window, int width, int height);
static void KeyCallbackForGLFW(GLFWwindow* window, int key, int scancode, int action, int mods);
static void LogGLParams(std::string file_name);
static double UpdateTime(double &fps, double int_fps_calc = 1.0);
static void get_shader_file_contents(const std::string& shader, char*& content);
static bool CheckShaderCompileStatus(GLuint shader_hdl, std::string diag_msg);
static bool CheckShaderProgramLinkStatus(GLuint program_hdl, std::string diag_msg);
static bool ParseOBJ(const std::string& path); //added by student.

static GLFWwindow* s_window_handle; // handle to both window and GL context
static std::string s_log_filename{ "debuglog.txt" }; // log file name
static int s_window_width = 1600, s_window_height = 900; // viewport dimensions
static double s_frames_per_second;
static bool s_window_close_flag = false;
static bool s_window_fullsize = false;


// everything required to define the geometry of the object ...
static std::vector<glm::vec3> allVertices;
//static std::vector<glm::vec3> allNormals; //<--- student added
static std::vector<int> allIndices; 
static std::vector<glm::vec3> allVertexNormals;
static glm::vec3 colorForAllVertices = glm::vec3(0.f, .25f, 0.f); 
static std::vector<glm::vec3> vtx_colors;//<--- added by student
static std::vector<glm::vec3> vtx_nrm_colors; //<--- added by student
static glm::vec3 colorForAllNormals = glm::vec3(1.f, 1.f, 1.f);

static glm::vec3 light_direction = { 1 / sqrt(3), 1 / sqrt(3), 20 };

static GLuint s_vao_hdl; // vertex array objects for FACET VERTICES 
static GLuint s_ebo_hdl; //element buffer object for indices.

//Added by student
static GLuint s_vbo_hdl; // vbo for vertices
static GLuint s_colorbuffer_hdl; // color buffer
static GLuint s_nvbo_hdl; 
static GLuint s_vtxncolorbuffer_hdl; // vertex normal color buffer 


static GLuint s_nvao_hdl; //vertex array objects for NORMALS
static GLuint s_nebo_hdl; //element array buffer objects for NORMALS

GLint model_matrix_loc;
GLint normal_matrix_loc;
GLint light_position_loc;

static float unitScale;
static glm::vec3 center;

//model selection variables
static char buf[128] = "";
static char* model[9] = { "cube", "gourd", "car", "dodecahedron", "teapot", "diamond",
 "pyramid", "sphere", "vase" };
static int model_size = sizeof(model) / sizeof(char*);


// everything required to update and display object ...
static GLuint s_shaderpgm_hdl; //the shader program object
static glm::vec3 s_scale_factors(5.f, 5.f, 5.f); // scale parameters
static glm::vec3 s_world_position(0.f, 0.f, -10.f); // position in world frame
static GLfloat s_angular_displacement = 0.f; // current angular displacement in degrees
static glm::vec3 s_orientation_axis = glm::normalize(glm::vec3(0.f, 1.f, 0.f)); // orientation axis
static glm::mat4 s_proj_mtx; // perspective xform - computed once for the whole scene
static glm::mat4 s_view_mtx; // view xform - again computed once for the whole scene
static glm::mat4 s_model_mtx; // model xform 
static glm::mat4 s_mvp_xform = glm::mat4(1.f); // model-world-view-clip transform matrix ...
static glm::mat4 s_nrm_mtx = glm::mat4(1.f);  //combines view and model matrix, transpose, inverse

static bool drawNormals;
static bool useVertexNormalColors;
static GLint u_is_vtx_nrm;
static GLint u_use_vtx_nrm_clr;

// For the editor
static float fov = 45.f;
static float near = 1.f;
static float far = 200.f; 


/******************************************************************************/
/*!
@fn     main()
@brief
	Initialize window handle, OpenGL, and run the main program loop.
*/
/******************************************************************************/
int main() {

  Init(); // very first update
  ImGui_ImplGlfwGL3_Init(s_window_handle, true);

  // Load Default Model
  LoadModel("assets/cube.obj"); //return value is not used....


  while (!glfwWindowShouldClose(s_window_handle))
  {
    Draw(); // render graphics task i
    Update(); // create graphics task i+1
  }

  ImGui_ImplGlfwGL3_Shutdown();
  Cleanup();
}

/******************************************************************************/
/*!
@fn     void CalculateVertexNormals()
@brief
        Calculate the normal of each vertex using the weighted average of
	all normals.
*/
/******************************************************************************/
void CalculateVertexNormals() {


    if (allVertexNormals.empty())
    {

        for (int i = 0; i < allVertices.size(); ++i)
        {
            glm::vec3 total_vec(0.0f);

            for (int j = 0; j < allIndices.size(); ++j)
            {
                //total_vec
                if (allIndices[j] == i)
                {
                    float angle;
                    glm::vec3 normal;
                    glm::vec3 orientation_QP;
                    glm::vec3 orientation_RP;
                    glm::vec3 edge_1;
                    glm::vec3 edge_2;

                    if (j % 3 == 0) //beginning of the face index
                    {
                        orientation_QP = allVertices[allIndices[j + 1]] - allVertices[allIndices[j]];
                        orientation_RP = allVertices[allIndices[j + 2]] - allVertices[allIndices[j]];

                        edge_1 = orientation_QP;
                        edge_2 = orientation_RP;
                    }
                    else if (j % 3 == 1) //middle of the face index
                    {
                        orientation_QP = allVertices[allIndices[j]] - allVertices[allIndices[j - 1]];
                        orientation_RP = allVertices[allIndices[j + 1]] - allVertices[allIndices[j - 1]];

                        edge_1 = allVertices[allIndices[j - 1]] - allVertices[allIndices[j]];
                        edge_2 = allVertices[allIndices[j + 1]] - allVertices[allIndices[j]];

                    }
                    else if (j % 3 == 2) //end of face index sequence
                    {
                        orientation_QP = allVertices[allIndices[j - 1]] - allVertices[allIndices[j - 2]];
                        orientation_RP = allVertices[allIndices[j]] - allVertices[allIndices[j - 2]];

                        edge_1 = allVertices[allIndices[j - 2]] - allVertices[allIndices[j]];
                        edge_2 = allVertices[allIndices[j - 1]] - allVertices[allIndices[j]];
                    }


                    angle = glm::degrees(acos((abs(glm::dot(edge_1, edge_2)) / 
                    (glm::length(edge_1) * glm::length(edge_2)))));
                    normal = glm::cross(orientation_QP, orientation_RP);
			
                    //The angle needs to be between the edges that *SHARE* the vertex.
                    total_vec += (angle * normal);
                }
            }

            allVertexNormals.push_back(allVertices[i]);
            allVertexNormals.push_back(allVertices[i] + glm::normalize(total_vec) * .06f); //point + vector equals another point

            //coloring by vertex normals
            vtx_nrm_colors.push_back(allVertices[i] + glm::normalize(total_vec));

        } //calculate the normals
    }
}

/******************************************************************************/
/*!
@fn     void DrawObject()
@brief
       Generate the buffers for the objects of the scene.
*/
/******************************************************************************/
void DrawObject() {

	// Create pointer for all indices  

  // set up buffer object with indices
  glGenBuffers(1, &s_ebo_hdl);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ebo_hdl);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndices.size() * sizeof(int), &allIndices[0], GL_STATIC_DRAW); //, GL_STATIC_DRAW);


	// create client-side vertex data for 1 triangle
  for (int i = 0; i < allVertices.size(); ++i) //don't want these to be uniform
  {
      vtx_colors.push_back(colorForAllVertices);
  }

  //Vertex Normals for light calculations
  glGenBuffers(1, &s_nvbo_hdl);
  glBindBuffer(GL_ARRAY_BUFFER, s_nvbo_hdl);
  glBufferData(GL_ARRAY_BUFFER, allVertexNormals.size() * sizeof(glm::vec3), &allVertexNormals[0], GL_STATIC_DRAW); 
   	
  // create vertex buffer object
	//@todo: IMPLEMENT ME
  glGenBuffers(1, &s_vbo_hdl);
  glBindBuffer(GL_ARRAY_BUFFER, s_vbo_hdl);
  glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(glm::vec3), &allVertices[0], GL_STATIC_DRAW); //, GL_STATIC_DRAW);

  glGenBuffers(1, &s_colorbuffer_hdl);
  glBindBuffer(GL_ARRAY_BUFFER, s_colorbuffer_hdl);
  glBufferData(GL_ARRAY_BUFFER, vtx_colors.size() * sizeof(glm::vec3), &vtx_colors[0], GL_STATIC_DRAW);


  glGenBuffers(1, &s_vtxncolorbuffer_hdl);
  glBindBuffer(GL_ARRAY_BUFFER, s_vtxncolorbuffer_hdl);
  glBufferData(GL_ARRAY_BUFFER, vtx_nrm_colors.size() * sizeof(glm::vec3), &vtx_nrm_colors[0], GL_STATIC_DRAW);

  // create vertex array object
  glGenVertexArrays(1, &s_vao_hdl);
  glBindVertexArray(s_vao_hdl);

  glBindBuffer(GL_ARRAY_BUFFER, s_vbo_hdl);
  
  //Vertex Indices == s_vbo_hdl
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s_ebo_hdl);

  glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);
  glEnableVertexAttribArray(0);

  //Colors == s_colorbuffer_hdl
  glBindBuffer(GL_ARRAY_BUFFER, s_colorbuffer_hdl);

  glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void*)0);
  glEnableVertexAttribArray(1);

  //Normal Colors
  glBindBuffer(GL_ARRAY_BUFFER, s_vtxncolorbuffer_hdl);
  glVertexAttribPointer(4, 3, GL_FLOAT, false, 0, (void*)0);
  glEnableVertexAttribArray(4);

  //Vertex Normals == s_nvbo_hdl
  glBindBuffer(GL_ARRAY_BUFFER, s_nvbo_hdl);

	// now, unbind triangle's VBO and VAO 
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void DrawNormals()
{     
    //uniforms associated with the normals.
     glUseProgram(s_shaderpgm_hdl);

     glUniform3f(glGetUniformLocation(s_shaderpgm_hdl, "vtx_nrm_clr"), colorForAllNormals.x, colorForAllNormals.y, colorForAllNormals.z);
     
     u_is_vtx_nrm = glGetUniformLocation(s_shaderpgm_hdl, "is_vtx_nrm");
     glUniform1i(u_is_vtx_nrm, 0);

     u_use_vtx_nrm_clr = glGetUniformLocation(s_shaderpgm_hdl, "use_vtx_nrm_clr");
     glUniform1i(u_use_vtx_nrm_clr, 0);

     glUseProgram(0);
   
     glGenBuffers(1, &s_nebo_hdl);
     glBindBuffer(GL_ARRAY_BUFFER, s_nebo_hdl);
     glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * allVertexNormals.size(), &allVertexNormals[0], GL_STATIC_DRAW);
     
     //bind vao and capture the vbos
     glGenVertexArrays(1, &s_nvao_hdl);
     glBindVertexArray(s_nvao_hdl);

     glBindBuffer(GL_ARRAY_BUFFER, s_nebo_hdl);

     glVertexAttribPointer(3, 3, GL_FLOAT, false, 0, (void*)0);
     glEnableVertexAttribArray(3);

     glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, (void*)0);
     glEnableVertexAttribArray(0);

     glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void*)0);
     glEnableVertexAttribArray(1);

     // create vertex array object
	
  	 // now, unbind triangle's VBO and VAO
     glBindVertexArray(0);
     glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool ParseOBJ(const std::string& path) {

    std::ifstream file;

    file.open(path, std::ios::in);

    if (!file) {
        std::cout << "had an error opening file!\n";
        return false;
    }
    std::string line;

    while (std::getline(file, line)) 
    {
        std::istringstream lineSStream(line);
        std::string type;
        lineSStream >> type;

        if (type == "v") {

            float x, y, z;

            lineSStream >> x >> y >> z;

            allVertices.emplace_back(glm::vec3( {x, y, z} ));
        }
        else if (type == "f") //this will only work for files that have delimiters '/' for their faces.
        {
            std::string str_f;
            while (lineSStream >> str_f)
            {
                std::istringstream ref(str_f);
                std::string vStr;
                std::getline(ref, vStr, '/');
                int v = atoi(vStr.c_str()) - 1;
                allIndices.push_back(v);
                std::getline(ref, vStr, '/');
                std::getline(ref, vStr, '/');
            }

        }
    }

    file.close();

    return true;


}

bool LoadModel(const std::string& path)
{
	// Read and store data of model --> PARSE OBJ FILE
	//@todo: IMPLEMENT ME

    //CALCULATING THE VERTEX NORMALS/////////////////////////////

    //for each vertex, and each face sharing that vertex.
    //angle between p,q,r and their edges (q - p), (r - p)
    //TIMES cross(q - p, r - p)
    allVertexNormals.clear();
    vtx_nrm_colors.clear();
    allVertices.clear();
    allIndices.clear();
    vtx_colors.clear();
    s_mvp_xform = glm::mat4(1.0f);


   s_scale_factors = glm::vec3(5.f, 5.f, 5.f); // scale parameters
   s_world_position = glm::vec3(0.f, 0.f, -10.f); // position in world frame
   s_angular_displacement = 0.f; // current angular displacement in degrees
   s_orientation_axis = glm::normalize(glm::vec3(0.f, 1.f, 0.f)); // orientation axis
   light_direction = { 1 / sqrt(3), 1 / sqrt(3), 20 };


   if (!ParseOBJ(path)) 
   {
       return false;
   }

   // Calculating Center and unit scale 
  glm::vec3 min_points(0.f);
  glm::vec3 max_points(0.f);

  for (int i = 0; i < allVertices.size(); ++i) 
  {
      min_points.x = std::min(min_points.x, allVertices[i].x);
      min_points.y = std::min(min_points.y, allVertices[i].y);
      min_points.z = std::min(min_points.z, allVertices[i].z);

      max_points.x = std::max(max_points.x, allVertices[i].x);
      max_points.y = std::max(max_points.y, allVertices[i].y);
      max_points.z = std::max(max_points.z, allVertices[i].z);
  }

  center.x = (max_points.x + min_points.x) / 2.0f;
  center.y = (max_points.y + min_points.y) / 2.0f;
  center.z = (max_points.z + min_points.z) / 2.0f;

  //view matrix
  s_view_mtx = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0), glm::vec3(0, 1, 0));

  //perspective matrix
  s_proj_mtx = glm::perspective(glm::radians(fov), ((float)s_window_width / (float)s_window_height), near, far);

  glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(s_angular_displacement), s_orientation_axis);
  glm::mat4 translate_world = glm::translate(glm::mat4(1.0f), s_world_position);
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), s_scale_factors);

  glm::mat4 update_mtx = translate_world * rotate * scale;

  s_mvp_xform = s_proj_mtx * s_view_mtx * update_mtx;


  // Resize object to (1, 1, 1) 
  unitScale = std::max({ glm::length(max_points.x - min_points.x),
  glm::length(max_points.y - min_points.y), glm::length(max_points.z - min_points.z)});

  //SRT thing -- model matrix.
  for (int i = 0; i < allVertices.size(); ++i) 
  {
      allVertices[i] =  (allVertices[i] - center) / unitScale;
  }
   CalculateVertexNormals();

   DrawObject();
   DrawNormals();
   return false;
}

/*
Initialize GLFW 3
Initialize GLEW
Initialize GL
Create log file
*/
void Init() {
  CreateLogFile(s_log_filename);

  glfwSetErrorCallback(ErrorCallbackForGLFW); // register GLFW error callback ...
  if (!glfwInit()) {  // start GL context and OS window using GLFW ...
    std::cerr << "ERROR: Could not start GLFW3" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::string gstr("GLFW version: "); gstr += glfwGetVersionString();
  WriteToFile(s_log_filename, gstr.c_str());

  // specify modern GL version ...
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // our framebuffer will consist of 24-bit depthbuffer and double buffered 24-bit RGB color buffer
  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  glfwWindowHint(GLFW_RED_BITS, 8);
  glfwWindowHint(GLFW_GREEN_BITS, 8);
  glfwWindowHint(GLFW_BLUE_BITS, 8);
  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

  glfwWindowHint(GLFW_SAMPLES, 4); // set multisample antialiasing sample count to 4

  GLFWmonitor* mon = nullptr;
  if (s_window_fullsize) { // full-screen window
    mon = glfwGetPrimaryMonitor();
    GLFWvidmode const* video_mode = glfwGetVideoMode(mon);
    s_window_width = video_mode->width; s_window_height = video_mode->height;
  }
  s_window_handle = glfwCreateWindow(s_window_width, s_window_height, "Model Loader", mon, nullptr);

  if (!s_window_handle) {
    std::cerr << "ERROR: Could not open window with GLFW3" << std::endl;
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }
  
  glfwSetWindowSizeCallback(s_window_handle, WindowResizeCallbackForGLFW);

  glfwMakeContextCurrent(s_window_handle);

  // don't wait for device's vertical sync for front and back buffers to be swapped
  glfwSwapInterval(0);

  glfwSetKeyCallback(s_window_handle, KeyCallbackForGLFW);

  // write title to window with current fps ...
  UpdateTime(s_frames_per_second, 1.0);
  std::stringstream sstr;
  sstr << std::fixed << std::setprecision(2) << "Model Loader: " << s_frames_per_second;
  glfwSetWindowTitle(s_window_handle, sstr.str().c_str());

  glewExperimental = GL_TRUE; // start GLEW extension library
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "Error: Unable to initialize GLEW " << glewGetErrorString(err) << std::endl;
    glfwDestroyWindow(s_window_handle); // destroy window and corresponding GL context
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  if (!GLEW_VERSION_4_3) { // check support for core GL 4.3 ...
    std::cout << "Error: Cannot access GL 4.3 API" << std::endl;
    glfwDestroyWindow(s_window_handle);
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }
  LogGLParams(s_log_filename);

  // set viewport - we'll use the entire window as viewport ...
  GLsizei fbwidth, fbheight;
  glfwGetFramebufferSize(s_window_handle, &fbwidth, &fbheight);
  glViewport(0, 0, fbwidth, fbheight);
	
  // read and compile shaders
  s_shaderpgm_hdl = glCreateProgram();

  const char* files[2] = { "Shaders/vertexShader.vert", "Shaders/FragmentShader.frag" };
  GLenum types[2] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
  GLuint shaders[2];
  for (int i = 0; i < 2; ++i) {
      std::ifstream in(files[i]);
      std::cout << "loading file " << files[i] << std::endl;
      std::string code;
      while (in) {
          std::string line;
          getline(in, line);
          code += line + "\n";
      }
      shaders[i] = glCreateShader(types[i]);
      const char* source = code.c_str();
      glShaderSource(shaders[i], 1, &source, 0);
      glCompileShader(shaders[i]);
      GLint status;
      glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &status);
      if (status != GL_TRUE) {
          std::string msg = "*** shader '" + std::string(files[i]) + "' failed to compile: ***\n";
          char buffer[1024];
          glGetShaderInfoLog(shaders[i], 1024, 0, buffer);
          std::cout << buffer << std::endl;
      }
      glAttachShader(s_shaderpgm_hdl, shaders[i]);
  }

  // link shaders
  glLinkProgram(s_shaderpgm_hdl);
  GLint status;
  glGetProgramiv(s_shaderpgm_hdl, GL_LINK_STATUS, &status);
  if (status != GL_TRUE)
      std::cout << "*** shader program failed to link ***" << std::endl;


  model_matrix_loc = glGetUniformLocation(s_shaderpgm_hdl, "uMVP");
  normal_matrix_loc = glGetUniformLocation(s_shaderpgm_hdl, "normalMatrix");
  light_position_loc = glGetUniformLocation(s_shaderpgm_hdl, "light_position");
  
  glDeleteShader(shaders[0]);
  glDeleteShader(shaders[1]);
  
}

void Draw() {

  glClearColor(0.2f, 0.2f, .6f, 1.f); // clear drawing surface with this color
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear framebuffer (color and depth)

  // write window title with current fps ...
  std::stringstream sstr;
  sstr << std::fixed << std::setprecision(2) << "Model Loader: " << s_frames_per_second;
  glfwSetWindowTitle(s_window_handle, sstr.str().c_str());

  // load vertex and fragment programs to corresponding processors --> idk what this means conceptually. -- X
	//@todo: IMPLEMENT ME
  glUseProgram(s_shaderpgm_hdl); //IDK if this is right based on the instructions. 


  // load "model-to-world-to-view-to-clip" matrix to uniform variable named "uMVP" in vertex shader -- CHECK
	//@todo: IMPLEMENT ME
 
  //GLint model_matrix = glGetUniformLocation(s_shaderpgm_hdl, "uMVP");
  glUniformMatrix4fv(model_matrix_loc, 1, false, &s_mvp_xform[0][0]);
  glUniformMatrix4fv(normal_matrix_loc, 1, false, &s_nrm_mtx[0][0]);
  glUniform4f(light_position_loc, light_direction.x, light_direction.y, light_direction.z, 1.0f);

  if (useVertexNormalColors) 
  {
      glUniform1i(u_use_vtx_nrm_clr, 1);
  }
  else 
  {
      glUniform1i(u_use_vtx_nrm_clr, 0);
  }

  // transfer vertices from server (GPU) buffers to vertex processer which must (at the very least)
  // compute the clip frame coordinates followed by assembly into triangles ...
  
  // bind VAO of triangle mesh   //--->shouldn't this already be referenced by the VAO????????
  //@todo: IMPLEMENT ME
  glBindVertexArray(s_vao_hdl);

  // Draw triangles
  //@todo: IMPLEMENT ME
  glDrawElements(GL_TRIANGLES, allIndices.size(), GL_UNSIGNED_INT, 0);


  // programming tip: always reset state - application will be easier to debug ...
 /* glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/
  glBindVertexArray(0);
  glUseProgram(0);

  if (drawNormals)
  {  
	  // use program to draw normals here
	  //@todo: IMPLEMENT ME
      glUseProgram(s_shaderpgm_hdl);

      glUniform1i(u_is_vtx_nrm, 1);

      glBindVertexArray(s_nvao_hdl);
      
      glDrawArrays(GL_LINES, 0, allVertexNormals.size());

      glUniform1i(u_is_vtx_nrm, 0);

      glUseProgram(0);
      glBindVertexArray(0);
  }
}

void Update() {
  // time between previous and current frame
  double delta_time = UpdateTime(s_frames_per_second, 1.0);
  // Update other events like input handling 
  glfwPollEvents();

  if (s_window_close_flag) {
    glfwSetWindowShouldClose(s_window_handle, 1);
  }
  // Update all game components using previously computed delta_time ...

    ImGui_ImplGlfwGL3_NewFrame();

  // to compute perspective transform, get the current width and height of viewport (in case
  // the user has changed its size) ...
  GLsizei fb_width, fb_height;
  glfwGetFramebufferSize(s_window_handle, &fb_width, &fb_height);

  float objPos[3] = { s_world_position.x, s_world_position.y, s_world_position.z };
  float objScale[3] = { s_scale_factors.x, s_scale_factors.y, s_scale_factors.z };
  float objAngle = s_angular_displacement;
  float objAxis[3] = { s_orientation_axis.x, s_orientation_axis.y, s_orientation_axis.z };
  float objColor[3] = { colorForAllVertices.x, colorForAllVertices.y, colorForAllVertices.z };
  float lightPosition[3]= { light_direction.x, light_direction.y, light_direction.z };


  int current_model = -1;
  bool is_selected = false;
  std::string the_model = "";


  // Draw your GUI here
  //@todo: IMPLEMENT ME
  ImGui::Begin("Model Manipulator!!!\n");

  if (ImGui::CollapsingHeader("Select Model"))
  {
      if (ImGui::ListBox("", &current_model, model, 9)) 
      {
          the_model = model[current_model];
          is_selected = true;
      }
  }

  ImGui::Text("");

  ImGui::Text("Type in Model");

  ImGui::InputText("", buf, sizeof(buf));
  if (ImGui::Button("Load Model"))
  {
      the_model = buf;
      is_selected = true;
  }

  ImGui::Text("");

  ImGui::Text("Positioning");
  ImGui::Checkbox("Draw Normals", &drawNormals);

  ImGui::DragFloat3("position", objPos, 0.05f, -50, 50);
  ImGui::SliderFloat3("scale", objScale, 0.10, 10);
  ImGui::DragFloat("angle", &objAngle, 0.5f, -360, 360);
  ImGui::DragFloat3("axis", objAxis, 0.5f, 1.0f, 0.0f);

  ImGui::Text("");

  ImGui::Text("Perspective");
  ImGui::DragFloat("fov", &fov, 0.1, 0, 100);
  ImGui::DragFloat("near", &near, 0.1, 0.1, 100);
  ImGui::DragFloat("far", &far, 0.1, 10, -100);

  ImGui::Text("");

  ImGui::Text("Light");
  ImGui::DragFloat3("Light Position", lightPosition, 0.1f, -100.0f, 100.0f);

  ImGui::Checkbox("Color with Vertex Normals", &useVertexNormalColors);

  ImGui::End();

  s_world_position.x = objPos[0];
  s_world_position.y = objPos[1];
  s_world_position.z = objPos[2];

  s_scale_factors.x = objScale[0];
  s_scale_factors.y = objScale[1];
  s_scale_factors.z = objScale[2];

  s_angular_displacement = objAngle;
  //s_angular_displacement[1] = objAngle[1];
  //s_angular_displacement[2] = objAngle[2];

  light_direction.x = lightPosition[0];
  light_direction.y = lightPosition[1];
  light_direction.z = lightPosition[2];

  s_orientation_axis.x = objAxis[0];
  s_orientation_axis.y = objAxis[1];
  s_orientation_axis.z = objAxis[2];
  // compute view and projection transforms once for entire scene ...
  //@todo: IMPLEMENT ME

   //view matrix
  s_view_mtx = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0), glm::vec3(0, 1, 0));

  //perspective matrix
  s_proj_mtx = glm::perspective(glm::radians(fov), ((float)s_window_width / (float)s_window_height), near, far);

  // compute model-view-projection transformation matrix (this was covered in CS 250) ...
  //@todo: IMPLEMENT ME
  glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), glm::radians(s_angular_displacement), s_orientation_axis);

  glm::mat4 translate_world = glm::translate(glm::mat4(1.0f), s_world_position);
  glm::mat4 user_scale = glm::scale(glm::mat4(1.0f), s_scale_factors);

  glm::mat4 update_mtx = translate_world * rotate * user_scale;

  s_mvp_xform = s_proj_mtx * s_view_mtx * update_mtx;

  s_nrm_mtx = glm::inverse(glm::transpose((s_view_mtx * update_mtx)));

  // render your GUI
  //@todo: IMPLEMENT ME
  // 
  ImGui::Render();

  // put the stuff we've been drawing onto the display
  glfwSwapBuffers(s_window_handle);

  if (is_selected) 
  {
      LoadModel("assets/" + the_model + ".obj");
  }
}

/*
Return resources acquired by GLFW, GLEW, OpenGL context back to system ...
*/
void Cleanup() {
  // delete all vbo's attached to s_vao_hdl
  GLint max_vtx_attrib = 0;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vtx_attrib);
  glBindVertexArray(s_vao_hdl);
  for (int i = 0; i < max_vtx_attrib; ++i) {
    GLuint vbo_handle = 0;
    glGetVertexAttribIuiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo_handle);
    if (vbo_handle > 0) {
      glDeleteBuffers(1, &vbo_handle);
    }
  }
  glBindVertexArray(s_nvao_hdl);
  for (int i = 0; i < max_vtx_attrib; ++i) {
    GLuint vbo_handle = 0;
    glGetVertexAttribIuiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vbo_handle);
    if (vbo_handle > 0) {
      glDeleteBuffers(1, &vbo_handle);
    }
  }

  glDeleteBuffers(1, &s_vao_hdl);
  glDeleteBuffers(1, &s_ebo_hdl);
  glDeleteBuffers(1, &s_nvao_hdl);
  glDeleteBuffers(1, &s_nebo_hdl);
  glDeleteBuffers(1, &s_vbo_hdl); 
  glDeleteBuffers(1, &s_colorbuffer_hdl);
  glDeleteBuffers(1, &s_nvbo_hdl);
  glDeleteBuffers(1, &s_vtxncolorbuffer_hdl);
  glDeleteProgram(s_shaderpgm_hdl);


  glfwDestroyWindow(s_window_handle);
  glfwTerminate();
}

/*
Template function that writes the values of one or more parameters to file specified by first parameter
Function will ensure that file is closed before returning!!!
*/
template <typename... Args>
bool WriteToFile(std::string file_name, Args&&... args) {
  std::ofstream ofs(file_name.c_str(), std::ofstream::app);
  if (!ofs) {
    std::cerr << "ERROR: could not open log file " << file_name << " for writing" << std::endl;
    return false;
  }
  int dummy[sizeof...(Args)] = { (ofs << std::forward<Args>(args) << ' ', 0)... };
  ofs << std::endl;
  ofs.close();
  return true;
}

/*
Start a new log file with current time and date timestamp followed by timestamp of application build ...
Function will ensure that file is closed before returning!!!
*/
bool CreateLogFile(std::string file_name) {
  std::ofstream ofs(file_name.c_str(), std::ofstream::out);
  if (!ofs) {
    std::cerr << "ERROR: could not open log file " << file_name << " for writing" << std::endl;
    return false;
  }

  std::time_t curr_time = time(nullptr); // get current time
  ofs << "OpenGL Application Log File - local time: " << std::ctime(&curr_time); // convert current time to C-string format
  ofs << "Build version: " << __DATE__ << " " << __TIME__ << std::endl << std::endl;
  ofs.close();
  return true;
}

/*
Logs GL parameters ...
*/
void LogGLParams(std::string file_name) {
  GLenum param_enums[] = {
    GL_VENDOR,                    // 0
    GL_RENDERER,
    GL_VERSION,
    GL_SHADING_LANGUAGE_VERSION,  // 3

    GL_MAJOR_VERSION,             // 4
    GL_MINOR_VERSION,
    GL_MAX_ELEMENTS_VERTICES,
    GL_MAX_ELEMENTS_INDICES,
    GL_MAX_GEOMETRY_OUTPUT_VERTICES,
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE,
    GL_MAX_DRAW_BUFFERS,
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
    GL_MAX_TEXTURE_IMAGE_UNITS,
    GL_MAX_TEXTURE_SIZE,
    GL_MAX_VARYING_FLOATS,
    GL_MAX_VERTEX_ATTRIBS,
    GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS, // 18

    GL_MAX_VIEWPORT_DIMS,         // 19

    GL_STEREO                     // 20
  };
  char const* param_names[] = {
    "GL_VENDOR",                    // 0
    "GL_RENDERER",
    "GL_VERSION",
    "GL_SHADING_LANGUAGE_VERSION",  // 3

    "GL_MAJOR_VERSION",             // 4
    "GL_MINOR_VERSION",
    "GL_MAX_ELEMENTS_VERTICES",
    "GL_MAX_ELEMENTS_INDICES",
    "GL_MAX_GEOMETRY_OUTPUT_VERTICES",
    "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
    "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
    "GL_MAX_DRAW_BUFFERS",
    "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
    "GL_MAX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_TEXTURE_SIZE",
    "GL_MAX_VARYING_FLOATS",
    "GL_MAX_VERTEX_ATTRIBS",
    "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_VERTEX_UNIFORM_COMPONENTS", // 18

    "GL_MAX_VIEWPORT_DIMS",         // 19

    "GL_STEREO"                     // 20
  };
  // C-strings for 1st four parameters
  WriteToFile(file_name, "GL version information and context parameters:");
  int i = 0;
  for (i = 0; i < 4; ++i) {
    WriteToFile(file_name, param_names[i], reinterpret_cast<char const*>(glGetString(param_enums[i])));
  }

  // one integer for next set of fifteen parameters
  for (; i < 19; ++i) {
    GLint val;
    glGetIntegerv(param_enums[i], &val);
    WriteToFile(file_name, param_names[i], val);
  }

  // two integers for next parameter
  GLint dim[2];
  glGetIntegerv(param_enums[19], dim);
  WriteToFile(file_name, param_names[19], dim[0], dim[1]);

  // bool for next parameter
  GLboolean flag;
  glGetBooleanv(param_enums[20], &flag);
  WriteToFile(file_name, param_names[20], static_cast<GLint>(flag));

  WriteToFile(file_name, "-----------------------------");
}

/*
This error callback is specifically called whenever GLFW encounters an error.
GLFW supplies an error code and a human-readable description that is written to standard output ...
*/
void ErrorCallbackForGLFW(int error, char const* description) {
  std::cerr << "GLFW Error id: " << error << " | description: " << description << std::endl;
}

/*
This callback function is called when the window is resized ...
*/
void WindowResizeCallbackForGLFW(GLFWwindow* window, int width, int height) {
  s_window_width = width; s_window_height = height;
  // Update any perspective matrices used here since aspect ratio might have changed
  // .

  // Update viewport
  GLsizei fbwidth, fbheight;
  glfwGetFramebufferSize(window, &fbwidth, &fbheight);
  glViewport(0, 0, fbwidth, fbheight);
}

/*
This callback function is called whenever a keyboard key is pressed or released ...
*/
void KeyCallbackForGLFW(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    s_window_close_flag = true;
  }
}

/*
This function is first called in Init() and once each game loop by Update(). It uses GLFW's time functions to:
1. compute interval in seconds between each frame and return this value
2. compute the frames per second every "int_fps_calc" seconds - the default
value of parameter int_fps_calc is 1.0
*/
double UpdateTime(double &fps, double int_fps_calc) {
  // determine time (in seconds) between previous and current frame ...
  static double prev_time = glfwGetTime();
  double curr_time = glfwGetTime();
  double delta_time = curr_time - prev_time; // time between frames
  prev_time = curr_time;

  // fps calculations
  static double count = 0.0; // number of game loop iterations
  static double start_time = glfwGetTime();
  double elapsed_time = curr_time - start_time;

  ++count;

  // Update fps at least every 10 seconds ...
  int_fps_calc = (int_fps_calc < 0.0) ? 0.0 : int_fps_calc;
  int_fps_calc = (int_fps_calc > 10.0) ? 10.0 : int_fps_calc;
  if (elapsed_time >= int_fps_calc) {
    fps = count / elapsed_time;
    start_time = curr_time;
    count = 0.0;
  }
  return delta_time;
}

void get_shader_file_contents(const std::string& shader, char*& content)
{
  FILE *file;
  size_t count = 0;

  if (shader.c_str() != NULL)
  {
    fopen_s(&file, shader.c_str(), "rt");
    if (file != NULL)
    {
      fseek(file, 0, SEEK_END);
      count = ftell(file);
      rewind(file);

      if (count > 0)
      {
        content = (char *)malloc(sizeof(char) * (count + 1));
        count = fread(content, sizeof(char), count, file);
        content[count] = '\0';
      }

      fclose(file);
    }
    else
    {
      std::cout << "nothing on file";
    }
  }
}

/*
1. Check compile status of shader source.
2. If the shader source has not successfully compiled, then print any diagnostic messages and return false to caller.
3. Otherwise, return true.
*/
static bool CheckShaderCompileStatus(GLuint shader_hdl, std::string diag_msg) {
  GLint result;
  glGetShaderiv(shader_hdl, GL_COMPILE_STATUS, &result);
  if (GL_FALSE == result) {
    GLint log_len;
    glGetShaderiv(shader_hdl, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
      char *error_log_str = new GLchar[log_len];
      GLsizei written_log_len;
      glGetShaderInfoLog(shader_hdl, log_len, &written_log_len, error_log_str);
      diag_msg = error_log_str;
      delete[] error_log_str;
    }
    return false;
  }
  return true;
}

/*
1. Check link status of program.
2. If the program has not successfully linked, then print any diagnostic messages and return false to caller.
3. Otherwise, return true.
*/
static bool CheckShaderProgramLinkStatus(GLuint program_hdl, std::string diag_msg) {
  GLint result;
  glGetProgramiv(program_hdl, GL_LINK_STATUS, &result);
  if (GL_FALSE == result) {
    GLint log_len;
    glGetProgramiv(program_hdl, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
      char *error_log_str = new GLchar[log_len];
      GLsizei written_log_len;
      glGetProgramInfoLog(program_hdl, log_len, &written_log_len, error_log_str);
      diag_msg = error_log_str;
      delete[] error_log_str;
    }
    return false;
  }
  return true;
}
