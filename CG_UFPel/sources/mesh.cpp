#include "mesh.hpp"
#include <objloader.hpp>
#include <vboindexer.hpp>
#include <GL/glew.h>

//Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//Constructor
Mesh::Mesh(
	std::vector<unsigned short> indices, 
	std::vector<glm::vec3> vertices, 
	std::vector<glm::vec2> uvs, 
	std::vector<glm::vec3> normals)
{
	printf("Creating a new mesh:\n\Vertices: %d\n\t Uvs: %d\n\tNormals: %d\n\tIndices: %d\n-----------------------------\n",
		vertices.size(), uvs.size(), normals.size(),indices.size());

	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
	this->indices = indices;
	

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);


	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	printf("Mesh Created\n\Vertices: %d\n\t Uvs: %d\n\tNormals: %d\n\tIndices: %d\n-----------------------------\n",
		indexed_vertices.size(), indexed_uvs.size(), indexed_normals.size(), this->indices.size());

	updateSize();
}

//Getters
std::vector<glm::vec3> * Mesh::getIndexedVertices() {
	return &indexed_vertices;
}

std::vector<glm::vec2> * Mesh::getIndexedUvs() {
	return &indexed_uvs;
}

std::vector<glm::vec3> * Mesh::getIndexedNormals() {
	return &indexed_normals;
}

std::vector<unsigned short> * Mesh::getIndices() {
	return &indices;
}

GLuint Mesh::getVertexBuffer()
{
	return vertexbuffer;
}

GLuint Mesh::getUvBuffer()
{
	return uvbuffer;
}

GLuint Mesh::getNormalBuffer()
{
	return normalbuffer;
}

GLuint Mesh::getElementBuffer()
{
	return elementbuffer;
}

//Size
void Mesh::updateSize() {

	GLfloat
		min_x, max_x,
		min_y, max_y,
		min_z, max_z;
	min_x = max_x = Mesh::indexed_vertices[0].x;
	min_y = max_y = Mesh::indexed_vertices[0].y;
	min_z = max_z = Mesh::indexed_vertices[0].z;


	for (int i = 0; i < Mesh::indexed_vertices.size(); i++) {
		if (Mesh::indexed_vertices[i].x < min_x) min_x = Mesh::indexed_vertices[i].x;
		if (Mesh::indexed_vertices[i].x > max_x) max_x = Mesh::indexed_vertices[i].x;
		if (Mesh::indexed_vertices[i].y < min_y) min_y = Mesh::indexed_vertices[i].y;
		if (Mesh::indexed_vertices[i].y > max_y) max_y = Mesh::indexed_vertices[i].y;
		if (Mesh::indexed_vertices[i].z < min_z) min_z = Mesh::indexed_vertices[i].z;
		if (Mesh::indexed_vertices[i].z > max_z) max_z = Mesh::indexed_vertices[i].z;
	}

	Mesh::size.x = max_x - min_x;
	Mesh::size.y = max_y - min_y;
	Mesh::size.z = max_z - min_z;

}

glm::vec3 Mesh::getSize() {
	return Mesh::size;
}

//Load

void Mesh::loadMesh() {

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute
		2,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(
		2,                                // attribute
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

}

void Mesh::cleanup() {
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
}

void Mesh::rebind() {
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);


//	glGenBuffers(1, &uvbuffer);
//	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
//	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);


//	glGenBuffers(1, &normalbuffer);
//	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
//	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well

	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
}

void Mesh::verifyMesh() {
	printf("\tIndices: %d\n\tVertices: %d\n\tUvs: %d\n\tNormals: %d\n", this->indices.size(), this->indexed_vertices.size(), this->indexed_uvs.size(), this->indexed_normals.size());
}