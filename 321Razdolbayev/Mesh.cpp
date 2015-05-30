#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>
#include <vector>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

//=========================================================

Mesh::Mesh() :
_primitiveType(GL_TRIANGLES),
_vao(0),
_numVertices(0),
_inited(false)
{
}

Mesh::Mesh(GLuint primType,
		   const std::vector<float>& vertices,
		   const std::vector<float>& normals,
		   const std::vector<float>& texcoords)
{
	init(primType, vertices, normals, texcoords);
}

void Mesh::makeSphere(float radius, int N)
{
	int M = N / 2;

	Buffer<float> vertices;
	Buffer<float> normals;
	Buffer<float> texcoords;
	for (int i = 0; i < M; i++)
	{
		float theta = (float)M_PI * i / M;
		float theta1 = (float)M_PI * (i + 1) / M;

		for (int j = 0; j < N; j++)
		{
			float phi = 2.0f * (float)M_PI * j / N + (float)M_PI;
			float phi1 = 2.0f * (float)M_PI * (j + 1) / N + (float)M_PI;

			//Первый треугольник, образующий квад
			vertices.addVec3(cos(phi) * sin(theta) * radius, sin(phi) * sin(theta) * radius, cos(theta) * radius);
			vertices.addVec3(cos(phi1) * sin(theta) * radius, sin(phi1) * sin(theta) * radius, cos(theta) * radius);
			vertices.addVec3(cos(phi1) * sin(theta1) * radius, sin(phi1) * sin(theta1) * radius, cos(theta1) * radius);

			normals.addVec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
			normals.addVec3(cos(phi1) * sin(theta), sin(phi1) * sin(theta), cos(theta));
			normals.addVec3(cos(phi1) * sin(theta1), sin(phi1) * sin(theta1), cos(theta1));

			texcoords.addVec2((float)j / N, 1.0f - (float)i / M);
			texcoords.addVec2((float)(j + 1) / N, 1.0f - (float)i / M);
			texcoords.addVec2((float)(j + 1) / N, 1.0f - (float)(i + 1) / M);

			_numVertices += 3;

			//Второй треугольник, образующий квад
			vertices.addVec3(cos(phi) * sin(theta) * radius, sin(phi) * sin(theta) * radius, cos(theta) * radius);
			vertices.addVec3(cos(phi1) * sin(theta1) * radius, sin(phi1) * sin(theta1) * radius, cos(theta1) * radius);
			vertices.addVec3(cos(phi) * sin(theta1) * radius, sin(phi) * sin(theta1) * radius, cos(theta1) * radius);

			normals.addVec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
			normals.addVec3(cos(phi1) * sin(theta1), sin(phi1) * sin(theta1), cos(theta1));
			normals.addVec3(cos(phi) * sin(theta1), sin(phi) * sin(theta1), cos(theta1));

			texcoords.addVec2((float)j / N, 1.0f - (float)i / M);
			texcoords.addVec2((float)(j + 1) / N, 1.0f - (float)(i + 1) / M);
			texcoords.addVec2((float)j / N, 1.0f - (float)(i + 1) / M);

			_numVertices += 3;
		}
	}

	init(GL_TRIANGLES, vertices, normals, texcoords);
}

void Mesh::makeCube(float size)
{
	Buffer<float> vertices;
	Buffer<float> normals;
	Buffer<float> texcoords;

	//front 1
	vertices.addVec3(size, -size, size);
	vertices.addVec3(size, size, size);
	vertices.addVec3(size, size, -size);

	normals.addVec3(1.0, 0.0, 0.0);
	normals.addVec3(1.0, 0.0, 0.0);
	normals.addVec3(1.0, 0.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 1.0);
	texcoords.addVec2(1.0, 0.0);

	//front 2
	vertices.addVec3(size, -size, size);
	vertices.addVec3(size, size, -size);
	vertices.addVec3(size, -size, -size);

	normals.addVec3(1.0, 0.0, 0.0);
	normals.addVec3(1.0, 0.0, 0.0);
	normals.addVec3(1.0, 0.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 0.0);
	texcoords.addVec2(0.0, 0.0);

	//left 1
	vertices.addVec3(-size, -size, size);
	vertices.addVec3(size, -size, size);
	vertices.addVec3(size, -size, -size);

	normals.addVec3(0.0, -1.0, 0.0);
	normals.addVec3(0.0, -1.0, 0.0);
	normals.addVec3(0.0, -1.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 1.0);
	texcoords.addVec2(1.0, 0.0);

	//left 2
	vertices.addVec3(-size, -size, size);
	vertices.addVec3(size, -size, -size);
	vertices.addVec3(-size, -size, -size);

	normals.addVec3(0.0, -1.0, 0.0);
	normals.addVec3(0.0, -1.0, 0.0);
	normals.addVec3(0.0, -1.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 0.0);
	texcoords.addVec2(0.0, 0.0);

	//top 1
	vertices.addVec3(-size, size, size);
	vertices.addVec3(size, size, size);
	vertices.addVec3(size, -size, size);

	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 1.0);
	texcoords.addVec2(1.0, 0.0);

	//top 2
	vertices.addVec3(-size, size, size);
	vertices.addVec3(size, -size, size);
	vertices.addVec3(-size, -size, size);

	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 0.0);
	texcoords.addVec2(0.0, 0.0);

	//back 1
	vertices.addVec3(-size, -size, size);
	vertices.addVec3(-size, size, -size);
	vertices.addVec3(-size, size, size);

	normals.addVec3(-1.0, 0.0, 0.0);
	normals.addVec3(-1.0, 0.0, 0.0);
	normals.addVec3(-1.0, 0.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 0.0);
	texcoords.addVec2(1.0, 1.0);

	//back 2
	vertices.addVec3(-size, -size, size);
	vertices.addVec3(-size, -size, -size);
	vertices.addVec3(-size, size, -size);

	normals.addVec3(-1.0, 0.0, 0.0);
	normals.addVec3(-1.0, 0.0, 0.0);
	normals.addVec3(-1.0, 0.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(0.0, 0.0);
	texcoords.addVec2(1.0, 0.0);

	//right 1
	vertices.addVec3(-size, size, size);
	vertices.addVec3(size, size, -size);
	vertices.addVec3(size, size, size);

	normals.addVec3(0.0, 1.0, 0.0);
	normals.addVec3(0.0, 1.0, 0.0);
	normals.addVec3(0.0, 1.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 0.0);
	texcoords.addVec2(1.0, 1.0);

	//right 2
	vertices.addVec3(-size, size, size);
	vertices.addVec3(-size, size, -size);
	vertices.addVec3(+size, size, -size);

	normals.addVec3(0.0, 1.0, 0.0);
	normals.addVec3(0.0, 1.0, 0.0);
	normals.addVec3(0.0, 1.0, 0.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(0.0, 0.0);
	texcoords.addVec2(1.0, 0.0);

	//bottom 1
	vertices.addVec3(-size, size, -size);
	vertices.addVec3(size, -size, -size);
	vertices.addVec3(size, size, -size);

	normals.addVec3(0.0, 0.0, -1.0);
	normals.addVec3(0.0, 0.0, -1.0);
	normals.addVec3(0.0, 0.0, -1.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(1.0, 0.0);
	texcoords.addVec2(1.0, 1.0);

	//bottom 2
	vertices.addVec3(-size, size, -size);
	vertices.addVec3(-size, -size, -size);
	vertices.addVec3(size, -size, -size);

	normals.addVec3(0.0, 0.0, -1.0);
	normals.addVec3(0.0, 0.0, -1.0);
	normals.addVec3(0.0, 0.0, -1.0);

	texcoords.addVec2(0.0, 1.0);
	texcoords.addVec2(0.0, 0.0);
	texcoords.addVec2(1.0, 0.0);

	init(GL_TRIANGLES, vertices, normals, texcoords);
}

void Mesh::makeScreenAlignedQuad()
{
	Buffer<float> vertices;

	//front 1
	vertices.addVec3(-1.0, 1.0, 0.0);
	vertices.addVec3(1.0, 1.0, 0.0);
	vertices.addVec3(1.0, -1.0, 0.0);

	//front 2
	vertices.addVec3(-1.0, 1.0, 0.0);
	vertices.addVec3(1.0, -1.0, 0.0);
	vertices.addVec3(-1.0, -1.0, 0.0);

	_numVertices = 6;

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	_vao = 0;
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	_inited = true;
}

void Mesh::makeGroundPlane(float size, float numTiles)
{
	Buffer<float> vertices;
	Buffer<float> normals;
	Buffer<float> texcoords;

	//front 1
	vertices.addVec3(-size, size, 0.0);
	vertices.addVec3(size, size, 0.0);
	vertices.addVec3(size, -size, 0.0);

	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);

	texcoords.addVec2(-numTiles, numTiles);
	texcoords.addVec2(numTiles, numTiles);
	texcoords.addVec2(numTiles, -numTiles);

	//front 2
	vertices.addVec3(-size, size, 0.0);
	vertices.addVec3(size, -size, 0.0);
	vertices.addVec3(-size, -size, 0.0);

	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);
	normals.addVec3(0.0, 0.0, 1.0);

	texcoords.addVec2(-numTiles, numTiles);
	texcoords.addVec2(numTiles, -numTiles);
	texcoords.addVec2(-numTiles, -numTiles);

	init(GL_TRIANGLES, vertices, normals, texcoords);
}

void Mesh::makeViewVolume(const glm::vec3& pos, const glm::vec3& center, const glm::vec3& up,
						  float viewAngle, float aspect, float nearPlane, float farPlane)
{
	using namespace glm;

	Buffer<float> vertices;
	// текстуры не нужны

    vec3 uz = normalize(center - pos);
    vec3 ux = normalize(cross(uz, up));
    vec3 uy = cross(ux, uz);

    float nearWidth = tan(viewAngle / 2.0f) * nearPlane;
    float nearHeight = nearWidth*aspect;

    float farWidth = tan(viewAngle / 2.0f) * farPlane;
    float farHeight = farWidth*aspect;

    // side planes
    vertices.addVec3(pos + uz*farPlane - ux*farWidth + uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane - ux*nearWidth + uy*nearHeight);
    vertices.addVec3(pos + uz*farPlane - ux*farWidth - uy*farHeight);

    vertices.addVec3(pos + uz*nearPlane - ux*nearWidth + uy*nearHeight);
    vertices.addVec3(pos + uz*farPlane - ux*farWidth - uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane - ux*nearWidth - uy*nearHeight);

    vertices.addVec3(pos + uz*farPlane - ux*farWidth + uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane - ux*nearWidth + uy*nearHeight);
    vertices.addVec3(pos + uz*farPlane + ux*farWidth + uy*farHeight);

    vertices.addVec3(pos + uz*nearPlane - ux*nearWidth + uy*nearHeight);
    vertices.addVec3(pos + uz*farPlane + ux*farWidth + uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane + ux*nearWidth + uy*nearHeight);

    vertices.addVec3(pos + uz*farPlane + ux*farWidth + uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane + ux*nearWidth + uy*nearHeight);
    vertices.addVec3(pos + uz*farPlane + ux*farWidth - uy*farHeight);

    vertices.addVec3(pos + uz*nearPlane + ux*nearWidth + uy*nearHeight);
    vertices.addVec3(pos + uz*farPlane + ux*farWidth - uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane + ux*nearWidth - uy*nearHeight);

    vertices.addVec3(pos + uz*farPlane - ux*farWidth - uy*farHeight);
    vertices.addVec3(pos + uz*farPlane + ux*farWidth - uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane - ux*nearWidth - uy*nearHeight);

    vertices.addVec3(pos + uz*farPlane + ux*farWidth - uy*farHeight);
    vertices.addVec3(pos + uz*nearPlane - ux*nearWidth - uy*nearHeight);
    vertices.addVec3(pos + uz*nearPlane + ux*nearWidth - uy*nearHeight);


    init(GL_TRIANGLES, vertices);
}

void Mesh::loadFromFile(const std::string& filename)
{
	const struct aiScene* scene = aiImportFile(filename.c_str(), aiProcess_Triangulate);

	if (!scene)
	{
		std::cerr << aiGetErrorString() << std::endl;
		return;
	}

	if (scene->mNumMeshes == 0)
	{
		std::cerr << "There is no meshes in file " << filename << std::endl;;
		return;
	}

	const struct aiMesh* mesh = scene->mMeshes[0];

	if (!mesh->HasPositions())
	{
		std::cerr << "This demo does not support meshes without positions\n";
		return;
	}

	if (!mesh->HasNormals())
	{
		std::cerr << "This demo does not support meshes without normals\n";
		return;
	}

	if (!mesh->HasTextureCoords(0))
	{
		std::cerr << "This demo does not support meshes without texcoords for texture unit 0\n";
		return;
	}

	_numVertices = mesh->mNumVertices;

	std::vector<float> vertices(_numVertices * 3);
	std::vector<float> normals(_numVertices * 3);
	std::vector<float> texcoords(_numVertices * 2);

	for (unsigned int i = 0; i < _numVertices; i++)
	{
		const aiVector3D* vp = &(mesh->mVertices[i]);

		vertices[i * 3 + 0] = (GLfloat)vp->x;
		vertices[i * 3 + 1] = (GLfloat)vp->y;
		vertices[i * 3 + 2] = (GLfloat)vp->z;

		const aiVector3D* normal = &(mesh->mNormals[i]);

		normals[i * 3 + 0] = (GLfloat)normal->x;
		normals[i * 3 + 1] = (GLfloat)normal->y;
		normals[i * 3 + 2] = (GLfloat)normal->z;

		const aiVector3D* tc = &(mesh->mTextureCoords[0][i]);

		texcoords[i * 2 + 0] = (GLfloat)tc->x;
		texcoords[i * 2 + 1] = (GLfloat)tc->y;
	}

	aiReleaseImport(scene);

	init(GL_TRIANGLES, vertices, normals, texcoords);
}

void Mesh::draw()
{
	glBindVertexArray(_vao);
	glDrawArrays(_primitiveType, 0, _numVertices);
}


void Mesh::init(GLuint primType,
				const std::vector<float>& vertices,
				const std::vector<float>& normals,
				const std::vector<float>& texcoords)
{
	std::vector<float> buffer(vertices.size() + normals.size() + texcoords.size());
	std::copy(vertices.begin(), vertices.end(), buffer.begin());
	std::copy(normals.begin(), normals.end(), buffer.begin() + vertices.size());
	std::copy(texcoords.begin(), texcoords.end(),
			  buffer.begin() + vertices.size() + normals.size());

	_numVertices = vertices.size()/3;
	_primitiveType = primType;

	GLuint vbo = 0;
	if (1 || !_inited)
		glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

	_vao = 0;
	if (1 || !_inited)
		glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	if (1 || !_inited)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(_numVertices * 3 * 4));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(_numVertices * 3 * 4 * 2));

	glBindVertexArray(0);
	_inited = true;
}

void Mesh::init(GLuint primType,
				const std::vector<float>& vertices)
{
	_numVertices = vertices.size()/3;
	_primitiveType = primType;

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	_vao = 0;
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	_inited = true;
}
