#pragma once
#ifndef MESHSIMPLIFICATION_HPP
#define MESHSIMPLIFICATION_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <queue>
#include <set>  
#include <stack>
#include <functional>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <glerror.hpp>
#include "Mesh.hpp"


//Define a custom data type to represent a vertx
class Vertex {
public:
	Vertex(std::uint32_t vert, std::uint32_t n, std::set<unsigned short> v) : vertice(vert), neighbours(n), vizinhos(v) {};

	std::uint32_t vertice;
	std::uint32_t neighbours;
	std::set<unsigned short> vizinhos;		//vizinhos no set, acesso ao maior com *(vizinhos.begin())
};
/*
//Define a custon data type to store the alteration dones to the mesh, so it can be "undone"
class Step {
public:
Step(unsigned short rPos, glm::vec3 rVert, unsigned short ePos, glm::vec3 eVert, std::vector<unsigned short> pInd) : replaced_position(rPos), replaced_vertice(rVert), erased_position(ePos), erased_vertice(eVert),previousIndices(pInd){};

unsigned short replaced_position;
glm::vec3 replaced_vertice;
unsigned short erased_position;
glm::vec3 erased_vertice;
std::vector<unsigned short> previousIndices;
};
*/
//Define a custon data type to store the alteration dones to the mesh, so it can be "undone"
class Step {
public:
	Step(std::vector<glm::vec3> pVert, std::vector<unsigned short> pInd) :previousVertices(pVert), previousIndices(pInd) {};

	std::vector<glm::vec3> previousVertices;
	std::vector<unsigned short> previousIndices;
};

//Define a comparator class for heap
struct CompareNeighbours {
	inline bool operator()(Vertex const &  p1, Vertex const & p2) {
		return (p1.neighbours < p2.neighbours);
	}
};

class MeshSimplification
{	
	//Heap used to store vertex in a defined order
	std::vector<Vertex> removeHeap;

	//Stack used to store operations
	std::stack<Step> alterations;


	//Set usado para definir os vizinhos
	std::set<unsigned short> vizinhosNewVert;
	
public:
	void MeshSimplification::reduce(std::vector<glm::vec3>& indexed_vertices, std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_normals, int& continuousMeshSimplification);
	void MeshSimplification::reconstruct(std::vector<glm::vec3>& indexed_vertices, std::vector<unsigned short>& indices, int& continuousMeshSimplification);

private:
	void calculaVizinhos(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::set<unsigned short>& vizinhosNewVert, std::vector<Vertex>& removeHeap);
	glm::vec3 escolheAresta(std::vector<glm::vec3>& indexed_vertices, std::vector<Vertex>& removeHeap);
	Step storesStep(std::pair <unsigned short, unsigned short> edge, std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short>& indices);
	bool undoStep(std::stack<Step>& alterations, std::vector<glm::vec3>& indexed_vertices, std::vector<unsigned short>& indices);
	Vertex collapse(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::vector<Vertex>& removeHeap, std::pair <unsigned short, unsigned short>& edge, glm::vec3& newVert, std::vector<glm::vec3>& indexed_normals, std::stack<Step>& alterations);
};
#endif