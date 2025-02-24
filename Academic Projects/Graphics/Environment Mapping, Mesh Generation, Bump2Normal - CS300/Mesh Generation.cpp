/******************************************************************************/
/*!
\file   mesh.cpp
\par    Purpose: Library for generating polygonal meshes.
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

#include "mesh.hpp"
//#include <algorithm>


/*  Function prototype(s) */
void BuildIndexBuffer(int stacks, int slices, Mesh &mesh);
void addVertex(Mesh &mesh, const Vertex &v);
void addIndex(Mesh &mesh, int index);
void ComputeTangentsBitangents(VertexBufferType &vertices, const IndexBufferType &indices);
void ComputeNormals(Mesh &mesh);
void ComputeUVs(Mesh &mesh);


/******************************************************************************/
/*!
\fn     Mesh CreatePlane(int stacks, int slices)
\brief
        Create a plane of dimension 1*1. 
        The plane x/y-coordinates range from -0.5 to 0.5.
        The plane z-coordinate is 0.
        The normal of the plane is z-vector.
        The uv of each vertex on the plane should allow repeating texture,
        instead of being capped between 0 and 1.
\param  stacks
        Number of stacks on the plane
\param  slices
        Number of slices on the plane
\return
        The generated plane
*/
/******************************************************************************/
Mesh CreatePlane(int stacks, int slices)
{
    Mesh mesh;

    for (int stack = 0; stack <= stacks; ++stack)
    {
        float row = (float)stack / stacks;

        for (int slice = 0; slice <= slices; ++slice)
        {
            float col = (float)slice / slices;

            Vertex v;

            v.pos = Vec3(col - 0.5f, 0.5f - row, 0.0f);
            v.nrm = Vec3(0.0f, 0.0f, 1.0f);
            v.uv = Vec2(slice, stacks-stack);

            addVertex(mesh, v);
        }
    }

    BuildIndexBuffer(stacks, slices, mesh);
    ComputeTangentsBitangents(mesh.vertexBuffer, mesh.indexBuffer);

    return mesh;
}


/******************************************************************************/
/*!
\fn     Mesh CreateCube(int height, int length, int width)
\brief
        Create a cube of dimension 1*1*1 that is subdivided into
        "height" stacks, "length" slices along the x-axis and
        "width" slices along the z-axis.
        The x/y/z-coordinates still range from -0.5 to 0.5.
\param  height
        Number of stacks on the cube
\param  length
        Number of slices along the x-axis of the cube
\param  width
        Number of slices along the z-axis of the cube
\return
        The generated cube
*/
/******************************************************************************/
//Mesh CreateCube(int height, int length, int width)
Mesh CreateCube(int length, int height, int width)
{
    /*  Initial planes and their transformations to form the cube faces */
    Mesh planeMesh[3];
    planeMesh[0] = CreatePlane(height, length);     /*  XY-plane, used for +Z and -Z faces */
    planeMesh[1] = CreatePlane(height, width);      /*  YZ-plane, used for +X and -X faces */
    planeMesh[2] = CreatePlane(width, length);      /*  XZ-plane, used for +Y and -Y faces */
    Mesh mesh;

    Vec3 const translateArray[] =
    {
        Vec3(+0.0f, +0.0f, +0.5f), // Z+
        Vec3(+0.0f, +0.0f, -0.5f), // Z-
        Vec3(+0.5f, +0.0f, +0.0f), // X+
        Vec3(-0.5f, +0.0f, +0.0f), // X-
        Vec3(+0.0f, +0.5f, +0.0f), // Y+
        Vec3(+0.0f, -0.5f, +0.0f), // Y-
    };

    Vec2 const rotateArray[] =
    {
        Vec2(+0.0f, +0.0f),        // Z+
        Vec2(+0.0f, +PI),          // Z-
        Vec2(+0.0f, +HALF_PI),     // X+
        Vec2(+0.0f, -HALF_PI),     // X-
        Vec2(-HALF_PI, +0.0f),     // Y+
        Vec2(+HALF_PI, +0.0f)      // Y-
    };


    /*  Transform the initial planes and use their transformed vertices to create vertices of the cube */
    /*  The vertices should be pushed into vertexBuffer while their indices are pushed into indexBuffer */
    int numIndices = 0;
    for (int i = 0; i < 6; i++)
    {
        Mat4 rotateXMat = Rotate(rotateArray[i][X], BASIS[X]);
        Mat4 rotateYMat = Rotate(rotateArray[i][Y], BASIS[Y]);
        Mat4 translateMat = Translate(translateArray[i]);

        Mat4 transformMat = translateMat * rotateYMat * rotateXMat;

        int planeIndex = i / 2;
        for (int j = 0; j < planeMesh[planeIndex].numVertices; j++)
        {
            Vertex v;
            v.pos = Vec3(transformMat * Vec4(planeMesh[planeIndex].vertexBuffer[j].pos, 1.0f));
            v.nrm = Vec3(transformMat * Vec4(planeMesh[planeIndex].vertexBuffer[j].nrm, 0.0f));
            v.uv = planeMesh[planeIndex].vertexBuffer[j].uv;

            mesh.vertexBuffer.push_back(v);
        }

        for (int j = 0; j < planeMesh[planeIndex].numIndices; j++)
        {
            mesh.indexBuffer.push_back(planeMesh[planeIndex].indexBuffer[j] + numIndices);
        }
        numIndices += planeMesh[planeIndex].numVertices;
    }

    ComputeTangentsBitangents(mesh.vertexBuffer, mesh.indexBuffer);

    /*  Compute the number of vertices, indices and triangles */
    mesh.numVertices = mesh.vertexBuffer.size();
    mesh.numIndices = mesh.indexBuffer.size();
    mesh.numTris = mesh.numIndices / 3;

    return mesh;
}


/******************************************************************************/
/*!
\fn     Mesh CreateSphere(int stacks, int slices)
\brief
        Create a sphere of radius 0.5.
        The center of the sphere is (0, 0, 0).
\param  stacks
        Number of stacks on the sphere
\param  slices
        Number of slices on the sphere
\return
        The generated sphere
*/
/******************************************************************************/
Mesh CreateSphere(int stacks, int slices)
{
    Mesh mesh;
    
    float radius = 0.5f;

    for (int stack = 0; stack <= stacks; ++stack)
    {
        float row = (float)stack / stacks;
        float beta = (float)HALF_PI - row * (float)PI;

        float cos_b = cosf(beta);
        float sin_b = sinf(beta);

        /*  Don't use a single vertex to store the two poles because we need multiple uv coords */
        for (int slice = 0; slice <= slices; ++slice)
        {
            float col = (float)slice / slices;
            float alpha = col * (float)TWO_PI;

            float cos_a = cosf(alpha);
            float sin_a = sinf(alpha);

            Vertex v;

            v.pos = Vec3(radius * cos_b * sin_a, radius * sin_b, radius * cos_b * cos_a);
            v.nrm = Normalize(Vec3(v.pos[0], v.pos[1], v.pos[2]));
            v.uv = Vec2(col, row);

            addVertex(mesh, v);
        }
    }

    BuildIndexBuffer(stacks, slices, mesh);
    ComputeTangentsBitangents(mesh.vertexBuffer, mesh.indexBuffer);

    return mesh;
}


/******************************************************************************/
/*!
\fn     void BuildIndexBuffer(int stacks, int slices, Mesh &mesh)
\brief
        Generate the index buffer for the mesh.
\param  stacks
        Number of stacks on the mesh
\param  slices
        Number of slices on the mesh
\param  mesh
        The mesh whose index buffer will be generated.
*/
/******************************************************************************/
void BuildIndexBuffer(int stacks, int slices, Mesh &mesh)
{
    int row0, row1, i0, i1, i2;
    int stride = slices + 1;    /*  We count both the first and last vertices since they have different uvs */

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            row0 = i * stride;
            row1 = row0 + stride;

            /*  First triangle */
            i0 = row0 + j;
            i1 = row1 + j;
            i2 = row0 + j + 1;

            /*  Ignore degenerate triangle */
            if (!DegenerateTri( mesh.vertexBuffer[i0].pos, 
                                mesh.vertexBuffer[i1].pos, 
                                mesh.vertexBuffer[i2].pos))
            {
                addIndex(mesh, i0);
                addIndex(mesh, i1);
                addIndex(mesh, i2);
            }


            /*  Second triangle */
            i0 = row0 + j + 1;
            i1 = row1 + j;
            i2 = row1 + j + 1;

            /*  Ignore degenerate triangle */
            if (!DegenerateTri( mesh.vertexBuffer[i0].pos,
                                mesh.vertexBuffer[i1].pos,
                                mesh.vertexBuffer[i2].pos))
            {
                addIndex(mesh, i0);
                addIndex(mesh, i1);
                addIndex(mesh, i2);
            }
        }
    }
}


/******************************************************************************/
/*!
\fn     void addVertex(Mesh &mesh, const Vertex &v)
\brief
        Add a vertex to a mesh
\param  mesh
        The mesh to be updated.
\param  v
        The vertex to be added.
*/
/******************************************************************************/
void addVertex(Mesh &mesh, const Vertex &v)
{
    mesh.vertexBuffer.push_back(v);
    ++mesh.numVertices;
}


/******************************************************************************/
/*!
\fn     void addIndex(Mesh &mesh, int index)
\brief
        Add an index to a mesh
\param  mesh
        The mesh to be updated.
\param  index
        The vertex index to be added.
*/
/******************************************************************************/
void addIndex(Mesh &mesh, int index)
{
    mesh.indexBuffer.push_back(index);
    ++mesh.numIndices;

    if (mesh.numIndices % 3 == 0)
        ++mesh.numTris;
}


/******************************************************************************/
/*!
\fn     void ComputeTangentsBitangents(VertexBufferType &vertices, const IndexBufferType &indices)
\brief
        Compute the tangent and bitangent at each vertex of the mesh
\param  vertices
        The vertex list of the mesh
\param  indices
        The index list of the mesh
*/
/******************************************************************************/
void ComputeTangentsBitangents(VertexBufferType &vertices, const IndexBufferType &indices)
{
    int numVertices = vertices.size();
    int numIndices = indices.size();

    int *AverageCounter = new int[numVertices];

    std::memset(AverageCounter, 0, numVertices*sizeof(int));

    for (int index = 0; index < numIndices; index += 3)
    {
        int p0 = indices[index + 0];
        int p1 = indices[index + 1];
        int p2 = indices[index + 2];

        Vertex &A = vertices[p0];
        Vertex &B = vertices[p1];
        Vertex &C = vertices[p2];

        // Get our two vectors across the triangles
        Vec3 P = B.pos - A.pos;
        Vec3 Q = C.pos - A.pos;

        // Get our texture components of those vectors
        Vec2 Puv = B.uv - A.uv;
        Vec2 Quv = C.uv - A.uv;

        float texCoord = Puv.x * Quv.y - Quv.x * Puv.y;

        // This would handle the case of a small (possibly zero)
        // reciprocal, just skip this vertex, Tan and Bin aren't incremented
        // and the counter isn't incremented.
        if (texCoord < EPSILON && texCoord > -EPSILON)
            continue;

        float reciprocal = 1.0f / texCoord;

        Vec3 Tangent((P * Quv.y - Q * Puv.y) * reciprocal);
        Vec3 Bitangent((Q * Puv.x - P * Quv.x) * reciprocal);

        A.tan += Tangent;
        B.tan += Tangent;
        C.tan += Tangent;

        A.bitan += Bitangent;
        B.bitan += Bitangent;
        C.bitan += Bitangent;

        ++AverageCounter[p0];
        ++AverageCounter[p1];
        ++AverageCounter[p2];
    }

    // For each vertex, divide by the number of triangles shared by it
    for (int vert = 0; vert < numVertices; ++vert)
    {
        if (AverageCounter[vert])
        {
            vertices[vert].tan /= static_cast<float>(AverageCounter[vert]);
            vertices[vert].bitan /= static_cast<float>(AverageCounter[vert]);
        }
    }

    delete[] AverageCounter;
}


bool readDataLine(char *lineBuf, int *lineNum, FILE *fp, int MAX_LINE_LEN);

/******************************************************************************/
/*!
\fn     bool LoadOBJMesh(Mesh &mesh, char *filename)
\brief
        Load a mesh from an OBJ file. This function supports reading vertex
        positions and normals, and faces containing any number of vertices.
        However, the mesh class currently assumes 3-vertex faces (i.e. triangles)
        only. So make sure that you triangulate the mesh before saving it as
        the OBJ file you want to load.
\param  mesh
        The mesh to be updated.
\param  index
        The vertex index to be added.
*/
/******************************************************************************/
Mesh LoadOBJMesh(char *filename)
{
    Mesh mesh;

    const int MAX_LINE_LEN = 1024;
    char lineBuf[MAX_LINE_LEN + 1];
    int numLines;

    FILE *fp;
    if (fopen_s(&fp, filename, "r") != 0)
    {
        std::cerr << "Failed to open " << filename << "\n";
        exit(1);
    }

    int posID = 0;

    while (!feof(fp))
    {
        if (readDataLine(lineBuf, &numLines, fp, MAX_LINE_LEN))
        {
            if (lineBuf[0] == 'v')
            {
                char dataType[MAX_LINE_LEN + 1];
                float x, y, z;
                sscanf_s(lineBuf, "%s %f %f %f", dataType, sizeof(dataType), &x, &y, &z);

                Vertex v;
                if (!strcmp(dataType, "v"))
                {
                    v.pos = Vec3(x, y, z);
                    if (posID >= mesh.numVertices)
                    {
                        mesh.vertexBuffer.push_back(v);
                        ++mesh.numVertices;
                    }
                    else
                        mesh.vertexBuffer[posID].pos = v.pos;

                    ++posID;
                }
            }
            else if (lineBuf[0] == 'f')
            {
                ++mesh.numTris;

                std::vector<char*> faceData;
                char *tokWS, *ptrFront, *ptrRear;
                char *ct;

                tokWS = strtok_s(lineBuf, " ", &ct);
                tokWS = strtok_s(NULL, " ", &ct);
                while (tokWS != NULL)
                {
                    faceData.push_back(tokWS);
                    tokWS = strtok_s(NULL, " ", &ct);
                }

                if (faceData.size() > 3)
                {
                    std::cerr << "Only triangulated mesh is accepted.\n";
                    exit(1);
                }

                for (int i = 0; i < (int)faceData.size(); i++)
                {
                    int vertNum;

                    ptrFront = strchr(faceData[i], '/');
                    if (ptrFront == NULL)
                    {
                        vertNum = atoi(faceData[i]) - 1;
                        mesh.indexBuffer.push_back(vertNum);
                        ++mesh.numIndices;
                    }
                    else
                    {
                        char *tokFront, *tokRear, *cF;
                        ptrRear = strrchr(faceData[i], '/');
                        tokFront = strtok_s(faceData[i], "/", &cF);
                        vertNum = atoi(tokFront) - 1;

                        if (ptrRear == ptrFront)
                        {
                            mesh.indexBuffer.push_back(vertNum);
                            ++mesh.numIndices;
                        }
                        else
                        {
                            if (ptrRear != ptrFront + 1)
                            {
                                tokRear = strtok_s(NULL, "/", &cF);
                            }

                            tokRear = strtok_s(NULL, "/", &cF);
                            mesh.indexBuffer.push_back(vertNum);
                            ++mesh.numIndices;
                        }
                    }
                }
            }

        }
    }

    if (fp)
        if (fclose(fp))
        {
            std::cerr << "Failed to close " << filename << "\n";
            exit(1);
        }

    ComputeNormals(mesh);
    ComputeUVs(mesh);

    return mesh;
}


/******************************************************************************/
/*!
\fn     void ComputeNormals(Mesh &mesh)
\brief
        Compute vertex normals for the mesh.
        For each vertex, this can be done by computing the normals of the 
        triangles that are adjacent to (i.e. containing) the vertex, then 
        averaging the triangle normals. 
        Note that you need to normalize the triangle normals, with our assumption 
        that the size of each triangle does not affect the averaged normal.
\param  mesh
        The input mesh, whose vertex normals (nrm) are to be computed.
*/
/******************************************************************************/
void ComputeNormals(Mesh &mesh)
{
    std::vector<int> vert_shared(mesh.numVertices, 0);

    for (int i = 0; i < mesh.numTris; ++i) 
    {
        int& j = mesh.indexBuffer[i * 3];
        int& j_1 = mesh.indexBuffer[i * 3 + 1];
        int& j_2 = mesh.indexBuffer[i * 3 + 2];

        Vec3 P = mesh.vertexBuffer[j].pos;
        Vec3 Q = mesh.vertexBuffer[j_1].pos;
        Vec3 R = mesh.vertexBuffer[j_2].pos;

        Vec3 normal = glm::normalize(glm::cross(Q - P, R - P));

        mesh.vertexBuffer[j].nrm += normal;
        mesh.vertexBuffer[j_1].nrm += normal;
        mesh.vertexBuffer[j_2].nrm += normal;

        ++vert_shared[j];
        ++vert_shared[j_1];
        ++vert_shared[j_2];
    }

    //average out the nrm by the number of normals 
    //calculated at that vertex.
    for (int i = 0; i < mesh.numVertices; ++i) 
    {
        mesh.vertexBuffer[i].nrm /= vert_shared[i];
    }
}


/******************************************************************************/
/*!
\fn     void ComputeUVs(Mesh &mesh)
\brief
        This function computes the vertex texture coordinates so that texture 
        mapping can be done using a cylinder for S-mapping and intermediate 
        surface normal for O-mapping. The intermediate cylinder�s main axis has 
        xz-coordinate to be the averaged xz-coordinate of the vertices, and 
        y-coordinate ranges from the minimal to the maximal y-coordinates of 
        the vertices. We only use the cylinder wrapping and ignore its bottom
        and top caps.
\param  mesh
        The input mesh, whose vertex UVs (uv) are to be computed.
*/
/******************************************************************************/
void ComputeUVs(Mesh &mesh)
{
    float x = 0, h = 0, z = 0, 
    ymin = 0, ymax = 0, 
    zmax = 0, xmax = 0, 
    max_angle = 0, min_angle = 0;

    int zero_count = 0;
    for (int i = 0; i < mesh.numVertices; ++i) 
    {
        Vec3 &vert_pos = mesh.vertexBuffer[i].pos;

        x += vert_pos.x;
        z += vert_pos.z;

        ymin = std::min(ymin, vert_pos.y);
        ymax = std::max(ymax, vert_pos.y);
    }

    x /= mesh.numVertices;
    z /= mesh.numVertices;

    for (int i = 0; i < mesh.numVertices; ++i)
    {
        Vec3& pos = mesh.vertexBuffer[i].pos;
        
        float v = (pos.y - ymin) / (ymax - ymin);
    
        float angle = atan2f(pos.z - z, pos.x - x);

        if (angle < 0) {

            angle += (TWO_PI); //range from [PI, PI] --> going in an opposite direction.
        }

        float u = (angle)/ (TWO_PI) ;

        mesh.vertexBuffer[i].uv.x = u;
        mesh.vertexBuffer[i].uv.y = v;
    }
}