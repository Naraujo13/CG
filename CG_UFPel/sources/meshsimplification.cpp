
#include "MeshSimplification.hpp"

void MeshSimplification::reduce(std::vector<glm::vec3>& indexed_vertices, std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_normals, int& continuousMeshSimplification) {
	//std::cout << "Entra no simplificar. Vértices: " << (*(M.getIndexedVertices())).size() << std::endl;
	//if (&M == NULL) {
	//	std::cout << "Error. Mesh reference is empty." << std::endl;
	//	return;
	//}

	//std::vector<glm::vec3> indexed_vertices = *M.getIndexedVertices();
	//std::vector<unsigned short> indices = *M.getIndices();
	//std::vector<glm::vec3> indexed_normals = *M.getIndexedNormals();
	

	if (indexed_vertices.size() <= 3) {
		std::cout << "Mesh only have 3 vertices. Cannot reduce anymore." << std::endl;
		continuousMeshSimplification = 0;
		return;
	}
	
	
	removeHeap.clear();
	vizinhosNewVert.clear();

	calculaVizinhos(indices, indexed_vertices, vizinhosNewVert, removeHeap);

	int i = 0;

	std::pair <unsigned short, unsigned short> edge;	//edge to be collapsed (d1 and d2)

	edge = std::make_pair(removeHeap.front().vertice, *(removeHeap.front().vizinhos.begin()));	//cria par com aresta a sofrer collapse
	glm::vec3 newVert = escolheAresta(indexed_vertices, removeHeap);

	vizinhosNewVert.clear();						//set de vizinhos do novo vértice

	alterations.push(storesStep(edge, indexed_vertices, indices));	//store the changes done in this step, so they can be undone if needed

	/*
	//Makes the transition "smooth"
	nt passos = 1000;
	glm::vec3 v1Diff = (newVert - indexed_vertices.at(edge.first)) / glm::vec3(passos, passos, passos);
	glm::vec3 v2Diff = (newVert - indexed_vertices.at(edge.second)) / glm::vec3(passos, passos, passos);
																	for (int i = 0; i < passos; i++) {
																	indexed_vertices.at(edge.first) += v1Diff;
																	indexed_vertices.at(edge.second) += v2Diff;

																	//Bind, //Draw


																	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);


																	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

																	// Draw the triangles !
																	glDrawElements(
																	GL_TRIANGLES,        // mode
																	indices.size(),      // count
																	GL_UNSIGNED_SHORT,   // type
																	(void*)0             // element array buffer offset
																	);

																	glDisableVertexAttribArray(0);
																	glDisableVertexAttribArray(1);
																	glDisableVertexAttribArray(2);
																	// Draw tweak bars
																	TwDraw();
																	}
																	*/
																		//Faz o collapse na aresta escolhida(edge), recebendo como retorno o novo vertex
	Vertex newVertex = collapse(indices, indexed_vertices, removeHeap, edge, newVert, indexed_normals, alterations);
	std::pop_heap(removeHeap.begin(), removeHeap.end(), CompareNeighbours());
	removeHeap.pop_back();

	//Remove vértices antigos da Heap e recalcula os vizinhos dos vértices alterados(not yet)s
	int flag = 0;
	i = 0;
	for (auto it = removeHeap.begin(); it != removeHeap.end() && flag != 1; ++it, i++) {
		if (it->vertice == edge.second) {
			removeHeap.erase(it);
			flag = 1;
		}
	}
	//adiciona novo Vertex na Heap
	removeHeap.push_back(newVertex);
	std::push_heap(removeHeap.begin(), removeHeap.end(), CompareNeighbours());
	
	//std::cout << "Sai do simplificar. Vértices: " << (*(M.getIndexedVertices())).size() << " || " << indexed_vertices.size() << std::endl;
}

void MeshSimplification::reconstruct(std::vector<glm::vec3>& indexed_vertices, std::vector<unsigned short>& indices, int& continuousMeshSimplification) {
	
	//if (&M == NULL) {
	//	std::cout << "Error. Mesh reference is empty." << std::endl;
	//	return;
	//}
	//std::vector<glm::vec3>& indexed_vertices = *M.getIndexedVertices(); 
	//std::vector<unsigned short>& indices = *M.getIndices();
	
	if (!undoStep(alterations, indexed_vertices, indices)) {
		std::cout << "Mesh is complete. There are no more steps to be undone." << std::endl;
		continuousMeshSimplification = 0;
	}
}

void MeshSimplification::calculaVizinhos(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::set<unsigned short>& vizinhosNewVert, std::vector<Vertex>& removeHeap) {
	std::make_heap(removeHeap.begin(), removeHeap.end(), CompareNeighbours());
	//std::cout << "Iniciando cálculo inicial dos vizinhos...\n";
	int i = 0;
	for (auto it1 = begin(indexed_vertices); it1 != end(indexed_vertices); ++it1, i++) {	//Para cada vértice
		vizinhosNewVert.clear();
		int j = 0;
		for (auto it2 = begin(indices); it2 != end(indices); ++it2, j++) {			//Varre o vetor procurando ocorrências em triângulos para adicioná-los aos vizinhos
			if (*it2 == i) {			//Ao achar ocorrência
										//printf("\tOcorrencia encontrada...\n");
				if (j % 3 == 0) {		//Se é o primeiro vertice do triangulo
					vizinhosNewVert.insert(*(it2 + 1));
					vizinhosNewVert.insert(*(it2 + 2));
				}
				else if (j % 3 == 1) {	//Se é o segundo vertice do triangulo;
					vizinhosNewVert.insert(*(it2 - 1));
					vizinhosNewVert.insert(*(it2 + 1));
				}
				else {						//Terceiro vértice do triangulo
					vizinhosNewVert.insert(*(it2 - 1));
					vizinhosNewVert.insert(*(it2 - 2));
				}

			}
		}


		vizinhosNewVert.erase(i);
		Vertex v(i, vizinhosNewVert.size(), vizinhosNewVert);
		removeHeap.push_back(v);
	}

	//PRINT DE DEBUG DA HEAP
	std::make_heap(removeHeap.begin(), removeHeap.end(), CompareNeighbours());
	/*	printf("Elementos do Heap:\n");
	for (auto it = begin(removeHeap); it != end(removeHeap); ++it) {
	printf("Vértice %d: %d vizinhos\n", it->vertice, it->neighbours);
	printf("Vizinhos: |");
	for (auto it2 = begin(it->vizinhos); it2 != end(it->vizinhos); ++it2) {
	printf(" %hu |", *it2);
	}
	printf("\n");
	}
	*/
}

glm::vec3 MeshSimplification::escolheAresta(std::vector<glm::vec3>& indexed_vertices, std::vector<Vertex>& removeHeap) {
	float x, y, z;
	x = (indexed_vertices.at(removeHeap.front().vertice).x + indexed_vertices.at(*(removeHeap.front().vizinhos.begin())).x) / 2;
	y = (indexed_vertices.at(removeHeap.front().vertice).y + indexed_vertices.at(*(removeHeap.front().vizinhos.begin())).y) / 2;
	z = (indexed_vertices.at(removeHeap.front().vertice).z + indexed_vertices.at(*(removeHeap.front().vizinhos.begin())).z) / 2;
	glm::vec3 newVert(x, y, z);		//novo vértice na posição "ideal"

	return newVert;
}

Step MeshSimplification::storesStep(std::pair <unsigned short, unsigned short> edge, std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short>& indices) {
	Step newStep(indexed_vertices, indices);
	return newStep;
}

bool MeshSimplification::undoStep(std::stack<Step>& alterations, std::vector<glm::vec3>& indexed_vertices, std::vector<unsigned short>& indices) {
	//std::cout << "Remoção na pilha. Tamanho atual: " << alterations.size() << std::endl;
	//getchar();
	if (alterations.empty())
		return false;

	//Gets last step from stack
	Step lastStep = alterations.top();
	//Undo indices changes (edges)
	indices.swap(lastStep.previousIndices);
	indexed_vertices.swap(lastStep.previousVertices);
	/*
	//Undo erased vertices changes (re-insert the removed vertice)
	indexed_vertices.insert(indexed_vertices.begin() + lastStep.erased_position, lastStep.erased_vertice);
	//Undo replaced vertices changes (re-replaces the replaced vertice)
	indexed_vertices.at(lastStep.replaced_position) = lastStep.replaced_vertice;
	*/
	alterations.pop();

	return true;
}

Vertex MeshSimplification::collapse(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::vector<Vertex>& removeHeap, std::pair <unsigned short, unsigned short>& edge, glm::vec3& newVert, std::vector<glm::vec3>& indexed_normals, std::stack<Step>& alterations) {
	//percorre vetor de indices criando novas arestas e deletando antigas (consertando o "buraco" criando novos triangulos)
	std::set<unsigned short> vizinhosNewVert;
	int i = 0;
	for (auto it = begin(indices); it != end(indices); ++it, i++) {
		if (*it == edge.first || *it == edge.second) {		//se é ao que será substituido
			if (i % 3 == 0) {	//posição 0 da tupl
								//Teste
				vizinhosNewVert.insert(*(it + 1));
				vizinhosNewVert.insert(*(it + 2));
				if (*it == edge.second)
					*it = edge.first;
			}
			else if (i % 3 == 1) {	//posição 1 da tupla
									//Teste
				vizinhosNewVert.insert(*(it - 1));
				vizinhosNewVert.insert(*(it + 1));
				if (*it == edge.second)
					*it = edge.first;
			}
			else {		//posição 2 da tupla
						//Teste
				vizinhosNewVert.insert(*(it - 2));
				vizinhosNewVert.insert(*(it - 1));
				if (*it == edge.second)
					*it = edge.first;
			}

		}
		//Atualiza referências no de índice
		if (*it > edge.second)
			*it = *it - 1;
	}
	//Exclui vértice 2
	indexed_vertices.erase(indexed_vertices.begin() + edge.second);

	//Remove problema de triangulos que possuiam os dois ângulos 

	i = 0;
	int flag;
	std::vector<Vertex>::iterator v;
	for (auto it = begin(indices); it != end(indices); it = it + 3, i++) {
		if (*it == *(it + 1) || *it == *(it + 2) || *(it + 1) == *(it + 2)) {
			if (*it == *(it + 1)) {
				for (v = begin(removeHeap); v != end(removeHeap) && v->vertice != *(it + 2); ++v);		//Procura referência do vértice "que não é o repetido", salvando-a em v

				flag = 0;
				for (auto it2 = removeHeap.front().vizinhos.begin(); it2 != removeHeap.front().vizinhos.end() && flag != 1; ++it2) {	//Percorre vizinhos procurando outro substituto
					if (*it2 != v->vertice && v->vizinhos.find(*it2) == v->vizinhos.end() || vizinhosNewVert.find(*it2) == vizinhosNewVert.end()) {		//Ao encontrar um que não seja vizinho de um deles, o substitui  REVISAR CONDIÇÂO
						*(it + 1) = *it2;
						flag = 1;
					}
				}
				if (flag == 0) {
					indices.erase(indices.begin() + i);
					indices.erase(indices.begin() + i);
					indices.erase(indices.begin() + i);
				}
			}
			else if (*it == *(it + 2) || *(it + 1) == *(it + 2)) {
				if (*it == *(it + 2))
					for (v = begin(removeHeap); v != end(removeHeap) && v->vertice != *(it + 1); ++v);		//Procura referência do vértice "que não é o repetido", salvando-a em v
				else if (*(it + 1) == *(it + 2))
					for (v = begin(removeHeap); v != end(removeHeap) && v->vertice != *(it); ++v);		//Procura referência do vértice "que não é o repetido", salvando-a em v

				int flag = 0;
				for (auto it2 = removeHeap.front().vizinhos.begin(); it2 != removeHeap.front().vizinhos.end() && flag != 1; ++it2) {	//Percorre vizinhos procurando outro substituto
					if (*it2 != v->vertice && v->vizinhos.find(*it2) == v->vizinhos.end() || vizinhosNewVert.find(*it2) == vizinhosNewVert.end()) {		//Ao encontrar um que não seja vizinho de um deles, o substitui  REVISAR CONDIÇÂO
						*(it + 2) = *it2;
						flag = 1;
					}
				}
				if (flag == 0) {
					indices.erase(indices.begin() + i);
					indices.erase(indices.begin() + i);
					indices.erase(indices.begin() + i);
				}
			}
		}
	}


	indexed_vertices.at(edge.first) = newVert;
	Vertex newVertex(edge.first, vizinhosNewVert.size(), vizinhosNewVert);

	/* Corrige normais 	*/
	//	glm::vec3 newNormal;
	//	newNormal.x = ((indexed_normals.at(edge.first).x - indexed_vertices.at(edge.first).x) + (indexed_normals.at(edge.second).x - indexed_vertices.at(edge.second).x))/2;
	//	newNormal.y = ((indexed_normals.at(edge.first).y - indexed_vertices.at(edge.first).y) + (indexed_normals.at(edge.second).y - indexed_vertices.at(edge.second).y)) / 2;
	//	newNormal.z = ((indexed_normals.at(edge.first).z - indexed_vertices.at(edge.first).z) + (indexed_normals.at(edge.second).z - indexed_vertices.at(edge.second).z)) / 2;

	//	std::cout << indexed_vertices.size() << " " << indexed_normals.size() << std::endl;
	//	indexed_normals.at(edge.first) = (indexed_normals.at(edge.first) + indexed_normals.at(edge.second)) / glm::vec3(2,2,2);
	//	indexed_normals.erase(indexed_normals.begin() + edge.second);
	//	std::cout << indexed_vertices.size() << " " << indexed_normals.size() << std::endl;

	return newVertex;
}
