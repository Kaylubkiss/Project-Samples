////
// File: LightRenderSoft.cpp
// 
// Name: Caleb Kissinger
// Course: CS200
// Date: 11/28/2022
// Instructor: Jason Hanson
// Fall 2022
// 
// 
// This defines all the functions defined in the LightRenderSoft class.
// Drawing using mostly software.
////

#include "LightRenderSoft.h"
#include <algorithm>
#include <iostream>

namespace cs200 
{

	cs200::LightRenderSoft::LightRenderSoft(cs200::Raster& r) : raster(r) //LOOK AT notes.txt
	{
		ndc_to_dev = cs200::translate(cs200::vector(-.5, -.5f)) * cs200::scale(raster.width() / 2, raster.height()/2) *
			cs200::translate(cs200::vector(1.0f, 1.0f)); 
	}

	void cs200::LightRenderSoft::clearFrame(const glm::vec4& color) 
	{
		glm::vec4 c = 255.0f * color;

		raster.setColor(c.r, c.g, c.b);

		for (int j = 0; j < raster.height(); ++j) 
		{
			raster.gotoPoint(0, j);
			for (int i = 0; i < raster.width(); ++i) 
			{	
				raster.writePixel();
				raster.incrementX();
			}
		}

	}

	cs200::LightRenderSoft::~LightRenderSoft(void) 
	{
	}

	int cs200::LightRenderSoft::loadTexture(const cs200::Bitmap& bmp) 
	{
		textures.push_back(&bmp);
		return textures.size() - 1;
	}

	void cs200::LightRenderSoft::unloadTexture(int texture_index) 
	{

		textures[texture_index] = nullptr;
	}

	void cs200::LightRenderSoft::setLightPosition(const glm::vec4& P) 
	{
		light_position = P;
	}


	void cs200::LightRenderSoft::setLightRadius(float r)
	{
		light_radius = r;
	}

	void cs200::LightRenderSoft::setLightFactor(float f) 
	{
		light_factor = f;
	}

	void cs200::LightRenderSoft::setAmbientFactor(float a)
	{
		ambient_factor = a;

	}
	void cs200::LightRenderSoft::setModeling(const glm::mat4& M) 
	{
		modeling = M;
	}

	void cs200::LightRenderSoft::setCamera(const cs200::Camera& cam) 
	{
		world_to_ndc = cs200::cameraToNDC(cam) * cs200::worldToCamera(cam);
	}

	int cs200::LightRenderSoft::loadMesh(const cs200::TexturedMesh& m)
	{
		meshes.push_back(&m);
		return (meshes.size()- 1);
	}

	void cs200::LightRenderSoft::unloadMesh(int mesh_index)
	{
		meshes[mesh_index] = nullptr;
	}


	void cs200::LightRenderSoft::displayFaces(int mesh_index, int texture_index) 
	{
		const cs200::TexturedMesh& mesh = *meshes[mesh_index]; //alias to this mesh
		const cs200::Bitmap& texture = *textures[texture_index];

		// vertex processing step: apply "vertex shader" to each mesh vertex
		device_verts.clear(); //avoid resizing vectors
		world_verts.clear();

		glm::mat4 transform = world_to_ndc * modeling;

		for (int i = 0; i < mesh.vertexCount(); ++i) 
		{
			//applying vertex shader
			const glm::vec4& position = mesh.vertexArray()[i]; //alias

			glm::vec4 gl_Position = transform * position; 

			//convert NDC to device space
			device_verts.push_back(ndc_to_dev * gl_Position);
			world_verts.push_back(modeling * position);

		}

		//face processing step
		for (int n = 0; n < mesh.faceCount(); ++n) 
		{
			//alias the face array.
			const cs200::TexturedMesh::Face& face = mesh.faceArray()[n]; 

			const glm::vec4& P = device_verts[face.index1];
			const glm::vec4& Q = device_verts[face.index2];
			const glm::vec4& R = device_verts[face.index3];

			const glm::vec4& Pmesh = world_verts[face.index1];
			const glm::vec4& Qmesh = world_verts[face.index2];
			const glm::vec4& Rmesh = world_verts[face.index3];

			//alias texture coordinates from array.
			const glm::vec2 &uvP = mesh.texcoordArray()[face.index1]; 
			const glm::vec2& uvQ = mesh.texcoordArray()[face.index2];
			const glm::vec2& uvR = mesh.texcoordArray()[face.index3];

			//find AABB
			float xmin = std::min( {P.x, Q.x, R.x});
			float ymin = std::min( {P.y, Q.y, R.y});
			float xmax = std::max({ P.x, Q.x, R.x });
			float ymax = std::max({ P.y, Q.y, R.y });

			//clip AABB to frame buffer
			int imin = std::max(0, int(ceil(xmin)));
			int jmin = std::max(0, int(ceil(ymin)));
			int imax = std::min(raster.width() - 1, int(floor(xmax)));
			int jmax = std::min(raster.height() - 1, int(floor(ymax)));

			//compute BC coord functions
			// --- 1st BC coord func ---
			const glm::vec4 O = cs200::point(0, 0);

			glm::vec4 vla = R - Q;
			glm::vec4 mla = cs200::vector(vla.y, -vla.x);
			float dla = glm::dot(mla, Q - O);
			glm::vec4 la_fcn(mla.x, mla.y, 0, -dla);
			float Nla = glm::dot(la_fcn, P);
			la_fcn /= Nla;

			//-- 2nd BC coord fun --
			glm::vec4 vmu = R - P;
			glm::vec4 mmu = cs200::vector(vmu.y, -vmu.x);
			float dmu = glm::dot(mmu, P - O);
			glm::vec4 mu_fcn(mmu.x, mmu.y, 0, -dmu);
			float Nmu = glm::dot(mu_fcn, Q);
			mu_fcn /= Nmu;


			//pixel processing step (iterate through pixels in clipped AABB)
			for (int j = jmin; j <= jmax; ++j) 
			{
				for (int i = imin; i <= imax; ++i)
				{
					//point containment
					glm::vec4 I = cs200::point(i, j);


					float laI = glm::dot(la_fcn, I);
					float muI = glm::dot(mu_fcn, I);
					float nuI = 1.0f - laI - muI;


					if (laI >= 0 && muI >= 0 && nuI >= 0)
					{
							//interpolate texture coordinates
							glm::vec2 vtexcoord = laI * uvP + muI * uvQ + nuI * uvR;
							glm::vec4 interp_world_pos = laI * Pmesh + muI * Qmesh + nuI * Rmesh;

							//apply fragment shader to determine pixel color
							glm::vec3 color = cs200::getColor(texture, vtexcoord[0], vtexcoord[1]);



							float fspot =
								std::min(static_cast<float>(pow(light_radius / (glm::distance(interp_world_pos, light_position)), 2)), 1.0f);

							float ftot = (ambient_factor + light_factor * fspot);

							glm::vec4 frag_color = glm::vec4(color, 1);

							frag_color *= ftot;

							frag_color.a = 1.0f;

							//this is to ensure that the fractional manipulation does not cause overflow.
							frag_color = glm::clamp(frag_color, 0.0f, 255.0f);

							//write pixel to frame buffer
							raster.setColor(frag_color.r, frag_color.g, frag_color.b);
							raster.gotoPoint(i, j); //an unoptimized traversal (multiplication is somewhat expensive).
							raster.writePixel();
					}
				


				}

			}


		}
	}



}