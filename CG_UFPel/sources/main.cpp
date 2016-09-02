// Include standard headers
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
GLFWwindow* g_pWindow;
unsigned int g_nWidth = 1024, g_nHeight = 768;

// Include AntTweakBar
#include <AntTweakBar.h>
TwBar *g_pToolBar;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <objloader.hpp>
#include <vboindexer.hpp>
#include <glerror.hpp>


//Define a custom data type to represent a vertx
class Vertex {
public:
	Vertex(std::uint32_t vert, std::uint32_t n, std::set<unsigned short> v) : vertice(vert), neighbours(n), vizinhos(v) {};
	
	std::uint32_t vertice;
	std::uint32_t neighbours;
	std::set<unsigned short> vizinhos;		//vizinhos no set, acesso ao maior com *(vizinhos.begin())
};

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



//Define a comparator class for heap
struct CompareNeighbours {
	inline bool operator()(Vertex const &  p1, Vertex const & p2) {
		return (p1.neighbours < p2.neighbours);
	}
};


/*	Função que atualiza os vizinhos do vértice na posição vertNumber do vetor vértices
*	Complexidade 3N, O(N)
*//*
bool updateNeighbours(uint16_t vertNumber, std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short> indices, std::vector<Vertex> removeHeap) {
	std::set<unsigned short> vizinhos;

	//Calcula número de vizinhos para o vértice fornecido
	for (auto it2 = begin(indices); it2 != end(indices); ++it2) {			//Varre o vetor procurando ocorrências em triângulos para adicioná-los aos vizinhos	O(N)
		if (*it2 == vertNumber) {			//Ao achar ocorrência
			if (*it2 % 3 == 0) {		//Se é o primeiro vertice do triangulo
				vizinhos.insert(*(it2 + 1));
				vizinhos.insert(*(it2 + 2));
			}
			else if (*it2 % 3 == 1) {	//Se é o segundo vertice do triangulo
				vizinhos.insert(*(it2 - 1));
				vizinhos.insert(*(it2 + 1));
			}
			else {						//Terceiro vértice do triangulo
				vizinhos.insert(*(it2 - 1));
				vizinhos.insert(*(it2 - 2));
			}
		}
	}

	glm::vec3 vert;
	//Recupera o vértice no vector
	uint16_t i = 0;
	for (auto it = begin(indexed_vertices); it != end(indexed_vertices); ++it, i++) {	//O(N)
		if (i == vertNumber) {
			vert = *it;

		}
	}
	//Atualiza heap
	for (auto it1 = begin(removeHeap); it1 != end(removeHeap); ++it1) {					//O(N)
		if (it1->vertice == vert) {
			it1->neighbours = vizinhos.size();
		}
	}

}
*/

void WindowSizeCallBack(GLFWwindow *pWindow, int nWidth, int nHeight) {

	g_nWidth = nWidth;
	g_nHeight = nHeight;
	glViewport(0, 0, g_nWidth, g_nHeight);
	TwWindowSize(g_nWidth, g_nHeight);
}

void calculaVizinhos(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::set<unsigned short>& vizinhosNewVert, std::vector<Vertex>& removeHeap) {
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

glm::vec3 escolheAresta(std::vector<glm::vec3>& indexed_vertices, std::vector<Vertex>& removeHeap) {
	float x, y, z;
	x = (indexed_vertices.at(removeHeap.front().vertice).x + indexed_vertices.at(*(removeHeap.front().vizinhos.begin())).x) / 2;
	y = (indexed_vertices.at(removeHeap.front().vertice).y + indexed_vertices.at(*(removeHeap.front().vizinhos.begin())).y) / 2;
	z = (indexed_vertices.at(removeHeap.front().vertice).z + indexed_vertices.at(*(removeHeap.front().vizinhos.begin())).z) / 2;
	glm::vec3 newVert(x, y, z);		//novo vértice na posição "ideal"

	return newVert;
}

Step storesStep(std::pair <unsigned short, unsigned short> edge, std::vector<glm::vec3> indexed_vertices, std::vector<unsigned short>& indices) {
	Step newStep(edge.first, indexed_vertices.at(edge.first), edge.second, indexed_vertices.at(edge.second), indices);
	return newStep;
}

bool undoStep(std::stack<Step>& alterations, std::vector<glm::vec3>& indexed_vertices, std::vector<unsigned short>& indices) {
	//std::cout << "Remoção na pilha. Tamanho atual: " << alterations.size() << std::endl;
	//getchar();
	if (alterations.empty())
		return false;
	
	//Gets last step from stack
	Step lastStep = alterations.top();
	//Undo indices changes (edges)
	indices.swap(lastStep.previousIndices);
	//Undo erased vertices changes (re-insert the removed vertice)
	indexed_vertices.insert(indexed_vertices.begin() + lastStep.erased_position, lastStep.erased_vertice);
	//Undo replaced vertices changes (re-replaces the replaced vertice)
	indexed_vertices.at(lastStep.replaced_position) = lastStep.replaced_vertice;
	alterations.pop();

	return true;
}


Vertex collapse(std::vector<unsigned short>& indices, std::vector<glm::vec3>& indexed_vertices, std::vector<Vertex>& removeHeap, std::pair <unsigned short, unsigned short>& edge, glm::vec3& newVert, std::stack<Step>& alterations) {
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
	return newVertex;
}


int main(void)
{
	int nUseMouse = 0;

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	g_pWindow = glfwCreateWindow(g_nWidth, g_nHeight, "CG UFPel", NULL, NULL);
	if (g_pWindow == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(g_pWindow);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	check_gl_error();//OpenGL error from GLEW

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(g_nWidth, g_nHeight);

	// Set GLFW event callbacks. I removed glfwSetWindowSizeCallback for conciseness
	glfwSetMouseButtonCallback(g_pWindow, (GLFWmousebuttonfun)TwEventMouseButtonGLFW); // - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetCursorPosCallback(g_pWindow, (GLFWcursorposfun)TwEventMousePosGLFW);          // - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetScrollCallback(g_pWindow, (GLFWscrollfun)TwEventMouseWheelGLFW);    // - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetKeyCallback(g_pWindow, (GLFWkeyfun)TwEventKeyGLFW);                         // - Directly redirect GLFW key events to AntTweakBar
	glfwSetCharCallback(g_pWindow, (GLFWcharfun)TwEventCharGLFW);                      // - Directly redirect GLFW char events to AntTweakBar
	glfwSetWindowSizeCallback(g_pWindow, WindowSizeCallBack);

	//create the toolbar
	g_pToolBar = TwNewBar("CG UFPel ToolBar");
	// Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
	double speed = 0.0;
	TwAddVarRW(g_pToolBar, "speed", TW_TYPE_DOUBLE, &speed, " label='Rot speed' min=0 max=2 step=0.01 keyIncr=s keyDecr=S help='Rotation speed (turns/second)' ");
	// Add 'bgColor' to 'bar': it is a modifable variable of type TW_TYPE_COLOR3F (3 floats color)
	vec3 oColor(0.0f);
	TwAddVarRW(g_pToolBar, "bgColor", TW_TYPE_COLOR3F, &oColor[0], " label='Background color' ");

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(g_pWindow, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(g_pWindow, g_nWidth / 2, g_nHeight / 2);

	// Dark blue background
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("F:/Downloads/Aula/CG/CG_UFPel/shaders/StandardShading.vertexshader", "F:/Downloads/Aula/CG/CG_UFPel/shaders/StandardShading.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID      = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID  = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Load the texture
	GLuint Texture = loadDDS("F:/Downloads/Aula/CG/CG_UFPel/mesh/uvmap.DDS");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("F:/Downloads/Aula/CG/CG_UFPel/mesh/suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	/* ------------ Trabalho 1 ----------------- * 
	 * -- Algoritmo de simplificação de Mesh --- *
	 * - Parte 1: Cálculo inicial de vizinhos ---*
	 * - Parte 2: Remoção do Vértice ----------- *
	 * - Parte 3: Collapse --------------------- *
	 * - Parte 4: Cálculo dos vizinhos alterados *
	 * ----------------------------------------- */

	//Define vecotre contendo a struct definida no início do código (Contendo o número que representa o vértice no vetor de indices(uint32) e o número de vizinhos(uint32).
	std::vector<Vertex> removeHeap;

	//Set usado para definir os vizinhos
	std::set<unsigned short> vizinhosNewVert;
	
	//Pilha usada para armazenar as alterações realizadas
	std::stack<Step> alterations;	

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime(), lastTime2 = glfwGetTime();
	int nbFrames    = 0;
	int simplify = 1;
	
	

	//Bind

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

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	int continuousMeshSimplification = 0;


	/* ---- Instructions to user ---- */
	std::cout << "-----------------------------------" << std::endl;
	std::cout << "Instruções:" << std::endl;
	std::cout << "\t1. Operações unitárias:" << std::endl;
	std::cout << "\t\t1.1. A tecla \"N\" fará a simplificação de 1 vértice quando pressionada." <<std::endl;
	std::cout << "\t\t1.2. A tecla \"B\" irá desfazer a simplificação de 1 vértice quando pressionada." << std::endl;
	std::cout << "\t2. Operações Contínuas:" << std::endl;
	std::cout << "\t\t2.1. A tecla \"Backspace\" irá colocar o programa num modo de simplificação contínuo." << std::endl;
	std::cout << "\t\t2.2. A tecla \"=\" irá colocar o programa num modo de desfazer a simplificação contínuo." << std::endl;
	std::cout << "\t\t2.3. tA tecla \"Space\" irá parar qualquer um dos dois modos contínuos anteriores." << std::endl;
	std::cout << "\t3. Modos de desenho:" << std::endl;
	std::cout << "\t\t3.1. A tecla \"P\" irá setar o modo de desenho para \"pontos\"." << std::endl;
	std::cout << "\t\t3.2. A tecla \"L\" irá setar o modo de desenho para \"linhas\"." << std::endl;
	std::cout << "\t\t3.3. A tecla \"F\" irá setar o modo de desenho para \"preenhcer\"." << std::endl;
	std::cout << "-----------------------------------" << std::endl;
	do{
        check_gl_error();
		// Measure speed
		double currentTime2 = glfwGetTime();



		//Sets continuous simplification or "un-simplifications". Backspace for simplfications, equal to undo it, space to stop both.
		if (glfwGetKey(g_pWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
			continuousMeshSimplification = 1;
		else if (glfwGetKey(g_pWindow, GLFW_KEY_EQUAL) == GLFW_PRESS)
			continuousMeshSimplification = -1;
		else if (glfwGetKey(g_pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
			continuousMeshSimplification = 0;
	
		if ((currentTime2 - lastTime2 >= 0.5 && indexed_vertices.size()>3 && (glfwGetKey(g_pWindow,GLFW_KEY_N) == GLFW_PRESS) && continuousMeshSimplification == 0) || (continuousMeshSimplification == 1 && indexed_vertices.size()>3)) {
			if (continuousMeshSimplification == 0)
				lastTime2 += 0.5;
			/* ------------ Trabalho 1 ----------------- *
			* -- Algoritmo de simplificação de Mesh --- *
			* - Parte 1: Cálculo inicial de vizinhos ---*
			* - Parte 2: Remoção do Vértice ----------- *
			* - Parte 3: Collapse --------------------- *
			* - Parte 4: Cálculo dos vizinhos alterados *
			* ----------------------------------------- */
			removeHeap.clear();
			vizinhosNewVert.clear();

			calculaVizinhos(indices, indexed_vertices, vizinhosNewVert, removeHeap);

			int i = 0;

			//std::cout << "Fim parte 1\n";
			/* --------------------------------------- *
			* ----------     Fim  Parte1    ---------- *
			*----------------------------------------- */

			/* --------------------------------------- *
			* ----------       Parte2      ----------- *
			*----------------------------------------- */
			//std::cout << "Início da parte2: escolha da aresta, remõção dos vértices e cálculo do novo.\n";
			std::pair <unsigned short, unsigned short> edge;	//edge to be collapsed (d1 and d2)

			edge = std::make_pair(removeHeap.front().vertice, *(removeHeap.front().vizinhos.begin()));	//cria par com aresta a sofrer collapse
			glm::vec3 newVert = escolheAresta(indexed_vertices, removeHeap);

			vizinhosNewVert.clear();						//set de vizinhos do novo vértice

			alterations.push(storesStep(edge, indexed_vertices, indices));	//store the changes done in this step, so they can be undone if needed

			/*		
			//Makes the transition "smooth"
			int passos = 1000;
			glm::vec3 v1Diff = (newVert - indexed_vertices.at(edge.first)) / glm::vec3(passos, passos, passos);
			glm::vec3 v2Diff = (newVert - indexed_vertices.at(edge.second)) / glm::vec3(passos, passos, passos);
			for (int i = 0; i < passos; i++) {
				indexed_vertices.at(edge.first) += v1Diff;
				indexed_vertices.at(edge.second) += v2Diff;
		
				//Bind, //Draw
		
			
					glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);


					glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

					// Draw tweak bars
					TwDraw();
			}
			 */


			//Faz o collapse na aresta escolhida(edge), recebendo como retorno o novo vertex
			Vertex newVertex = collapse(indices, indexed_vertices, removeHeap, edge, newVert, alterations);
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

			/* -------------------------------------- *
			* ---------- Fim do trabalho 1 -----------*
			*-----------------------------------------*/


			//Bind

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

			// Get a handle for our "LightPosition" uniform
			glUseProgram(programID);
			LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

			//End Bind
		}
		else if ((currentTime2 - lastTime2 >= 0.5 && (glfwGetKey(g_pWindow, GLFW_KEY_N) == GLFW_PRESS) && indexed_vertices.size()<=3 && continuousMeshSimplification == 0) || (continuousMeshSimplification == 1 && indexed_vertices.size()<3)) {
			std::cout << "Mesh only have 3 vertices. Cannot simplify anymore." << std::endl;
			continuousMeshSimplification = 0;
		}
		else if ((currentTime2 - lastTime2 >= 0.5 && !alterations.empty() && (glfwGetKey(g_pWindow, GLFW_KEY_B) == GLFW_PRESS) && continuousMeshSimplification == 0) || (continuousMeshSimplification == -1 && !alterations.empty())) {
			if (continuousMeshSimplification == 0)
				lastTime2 += 0.5;
			if (!undoStep(alterations, indexed_vertices, indices)) {
				std::cout << "Error. Stack is empty." << std::endl;
				continuousMeshSimplification = 0;
			}

			//Bind

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

			// Get a handle for our "LightPosition" uniform
			glUseProgram(programID);
			LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

			//End Bind
		}
		else if ((alterations.empty() && (glfwGetKey(g_pWindow, GLFW_KEY_B) == GLFW_PRESS) && continuousMeshSimplification == 0) || ((continuousMeshSimplification == -1 && alterations.empty()))) {
			std::cout << "Mesh is complete. There are no steps to be undone." << std::endl;
			continuousMeshSimplification = 0;
		}

		//Set the draw mode. L to lines, P to points and F to fill.
		if(glfwGetKey(g_pWindow, GLFW_KEY_L) == GLFW_PRESS)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if ((glfwGetKey(g_pWindow, GLFW_KEY_P) == GLFW_PRESS))
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else if ((glfwGetKey(g_pWindow, GLFW_KEY_F) == GLFW_PRESS))
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //use the control key to free the mouse
		if (glfwGetKey(g_pWindow, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
			nUseMouse = 0;
		else
			nUseMouse = 1;

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\tContious state: %d\n", 1000.0 / double(nbFrames), continuousMeshSimplification);
			nbFrames  = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(nUseMouse, g_nWidth, g_nHeight);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix       = getViewMatrix();
		glm::mat4 ModelMatrix      = glm::mat4(1.0);
		glm::mat4 MVP              = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

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

		// Swap buffers
		glfwSwapBuffers(g_pWindow);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(g_pWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(g_pWindow) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Terminate AntTweakBar and GLFW
	TwTerminate();
	glfwTerminate();

	return 0;
}

