/* ---------------- Author: Nícolas Oreques de Araujo ---------------- */

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
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

//custom includes
#include "meshsimplification.hpp"
#include "model.hpp"
#include "modelManager.hpp"

using namespace glm;

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <objloader.hpp>
#include <vboindexer.hpp>
#include <glerror.hpp>

// Create an internal enum
typedef enum { MODEL, CAMERA } ACTIVE_TYPE;
// A variable for the current selection - will be updated by ATB
ACTIVE_TYPE m_currentActive = MODEL;

// Create an internal enum to name the meshes
typedef enum { SUZANNE, CUBE, GOOSE } MESH_TYPE;
// A variable for the current selection - will be updated by ATB
MESH_TYPE m_currentMesh = SUZANNE;

//Input variables
//Transformation auxiliaries
struct translation t;
struct rotation r;
struct scale s;
struct shear h;
struct bezier b;
struct bspline l;
struct rotationAP p, cp;
struct extraProjection3D p3D;
struct cameraLookAt lk;
glm::vec3 newModelPos(0);
//Cam var
glm::vec3 newCamPos(0, 0, 5), newCamUp(0, 1, 0), newCamSight(0, 0, -1);
float fieldOfView = 60.0f, aspectRatio = 0.0f, near = 5.0f, far = 10.0;
int hAux = 4.0f, vAux = 3.0f;
//Linear path var
glm::vec3 linearPoint(0);
std::vector<glm::vec3>linearPoints;
double linearTime = 0.0f;
int numLinearPoints = 1;
//Composition var
double compositionTime = 0.0f;
//Número de elementos
int numModelos = 4;//Meshes
int numCameras = 1;//Cameras

//Control variables
int currentShaderProgramID = 0;
int currentModelID = 0;
int currentCameraID = 0;
int nUseMouse = 0;


void printInstructions() {
	/* ---- Instructions to user ---- */
	std::cout << "-----------------------------------" << std::endl;
	std::cout << "Instrucoes:" << std::endl;
	std::cout << "\t1. Operacoes unitarias:" << std::endl;
	std::cout << "\t\t1.1. A tecla \"N\" fara a simplificacao de 1 vertice quando pressionada." << std::endl;
	std::cout << "\t\t1.2. A tecla \"B\" ira desfazer a simplificacao de 1 vertice quando pressionada." << std::endl;
	std::cout << "\t2. Operacoes Continuas:" << std::endl;
	std::cout << "\t\t2.1. A tecla \"Backspace\" ira colocar o programa num modo de simplificacao continuo." << std::endl;
	std::cout << "\t\t2.2. A tecla \"=\" ira colocar o programa num modo de desfazer a simplificacao continuo." << std::endl;
	std::cout << "\t\t2.3. tA tecla \"Space\" ira parar qualquer um dos dois modos continuos anteriores." << std::endl;
	std::cout << "\t3. Modos de desenho:" << std::endl;
	std::cout << "\t\t3.1. A tecla \"P\" ira setar o modo de desenho para \"pontos\"." << std::endl;
	std::cout << "\t\t3.2. A tecla \"L\" ira setar o modo de desenho para \"linhas\"." << std::endl;
	std::cout << "\t\t3.3. A tecla \"F\" ira setar o modo de desenho para \"preenhcer\"." << std::endl;
	std::cout << "\t4. Movimentação da câmera:" << std::endl;
	std::cout << "\t\t4.1. A tecla \"Control\" libera o mouse." << std::endl;
	std::cout << "\t\t4.2. As setas direcionais podem ser usadas para movimentao." << std::endl;
	std::cout << "\t5. Controle Geral:" << std::endl;
	std::cout << "\t\t5.1. O modelo ativo no momento pode ser escolhido atraves do campo da interface ou das teclas numericas." << std::endl;
	std::cout << "\t\t5.2. A tecla \"Insert\" adiciona um novo modelo utilizando a mesh e a posicao inicial selecionadas na interface" << std::endl;
	std::cout << "\t\t5.3. A tecla \"Delete\" ira resetar todas as entradas da interface." << std::endl;
	std::cout << "\t6. Transformacoes sobre modelos:" << std::endl;
	std::cout << "\t\t6.1. A tecla \"T\" adiciona uma translacao com os parametros selecionados na interface." << std::endl;
	std::cout << "\t\t6.2. A tecla \"S\" adiciona uma escala com os parametros selecionados na interface." << std::endl;
	std::cout << "\t\t6.3. A tecla \"R\" adiciona uma rotacao com os parametros selecionados na interface." << std::endl;
	std::cout << "\t\t6.4. A tecla \"C\" adiciona uma transformacao composta usando os parametros de rotacao e translacao, e o tempo pra realizar a animacao composta." << std::endl;
	std::cout << "\t\t6.5. A tecla \"H\" adiciona uma transformacao de shear com os parametros selecionados na interface" << std::endl;
	std::cout << "\t\t6.6. A tecla \"A\" adiciona uma rotacao ao redor de ponto com os parametros selecionados na interface" << std::endl;
	std::cout << "\t\t6.7. A tecla \"B\" adiciona uma curva B-Spline com os parametros selecionados na interface" << std::endl;
	std::cout << "\t\t6.8. A tecla \"Z\" adiciona uma curva Bezier com os parametros selecionados na interface" << std::endl;
	std::cout << "\t\t6.9. A tecla \"D\" adiciona uma projecao 3Dom os parametros selecionados na interface." << std::endl;
	std::cout << "\t\t6.10. A tecla \"U\" ativa a realizacao de todas as transformacoes na fila." << std::endl;
	std::cout << "\t\t Todos os comandos da secao 6 serao realizados para o modelo ativo no momento." << std::endl;
	std::cout << "\t Ao manter-se a tecla \"Left Shift\" pressionada, sera impresso um print de performance a cada 1 segundo." << std::endl;
	std::cout << "----------------------------------" << std::endl;
}

void controlCounterVariables(ModelManager& manager) 
{
	//Limita ShaderID ao tamanho do vetor
	if (currentShaderProgramID > (*manager.getShaders()).size() - 1)
		currentShaderProgramID = (*manager.getShaders()).size() - 1;
	//Limita cameraID ao tamanho do vetor
	if (currentCameraID > (*manager.getCameras()).size() - 1)
		currentCameraID = (*manager.getCameras()).size() - 1;
	//Limita ModelID ao tamanho do vetor
	if (currentModelID > (*manager.getModels()).size() - 1)
		currentModelID = (*manager.getModels()).size() - 1;
	//Numero de pontos automatico
	if (numLinearPoints != linearPoints.size() + 1)
		numLinearPoints = linearPoints.size() + 1;
}

void initialiseTransformationVariables() {
	t.translationVec = glm::vec3(0, 0, 0);
	t.time = 1.0f;
	s.scaleVec = glm::vec3(1);
	s.time = 1.0f;
	r.rotationVec = glm::vec3(1, 0, 0);
	r.rotationDegrees = 0.0f;
	r.time = 1.0f;
	h.shearVec = glm::vec3(0, 0, 0);
	h.time = 1.0f;
	b.controlPoints[0] = glm::vec3(0);
	b.controlPoints[1] = glm::vec3(0);
	b.controlPoints[2] = glm::vec3(0);
	b.time = 1.0f;
	l.controlPoints[0] = glm::vec3(-3, 3, 10);
	l.controlPoints[1] = glm::vec3(-3, -3, 10);
	l.controlPoints[2] = glm::vec3(3, -3, 10);
	l.controlPoints[3] = glm::vec3(3, 3, 10);
	l.time = 1.0f;
	p.rotationAngle = 360.0f;
	p.point = glm::vec3(3, 0, 0);
	p.time = 1.0f;
	cp.rotationAngle = 360.0f;
	cp.rotationAxis = glm::vec3(1, 0, 0);
	cp.point = glm::vec3(3, 0, 0);
	cp.time = 1.0f;
	p3D.projVector = glm::vec3(0, 0, 2);
	p3D.time = 0.0f;
	lk.eye = glm::vec3(0, 0, 5);
	lk.center = glm::vec3(0, 0, -5);
	lk.up = glm::vec3(0, 1, 0);
}

void meshSimplificationInput(
	int& continuousMeshSimplification,
	long double& currentTime2,
	long double& lastTime2,
	MeshSimplification& MS,
	int& currentModelID,
	ModelManager& manager) {
	//Sets continuous simplification or "un-simplifications". Backspace for simplfications, equal to undo it, space to stop both.
	if (glfwGetKey(g_pWindow, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		continuousMeshSimplification = 1;
	else if (glfwGetKey(g_pWindow, GLFW_KEY_EQUAL) == GLFW_PRESS)
		continuousMeshSimplification = -1;
	else if (glfwGetKey(g_pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
		continuousMeshSimplification = 0;

	//Trabalho 1 - Mesh Simplification
	if ((currentTime2 >= lastTime2 + 0.5 && (glfwGetKey(g_pWindow, GLFW_KEY_F10) == GLFW_PRESS) && continuousMeshSimplification == 0) || continuousMeshSimplification == 1) {

		lastTime2 = glfwGetTime();
		/* ------------ Trabalho 1 ----------------- *
		* -- Algoritmo de simplificação de Mesh --- *
		* - Parte 1: Cálculo inicial de vizinhos ---*
		* - Parte 2: Remoção do Vértice ----------- *
		* - Parte 3: Collapse --------------------- *
		* - Parte 4: Cálculo dos vizinhos alterados *
		* ----------------------------------------- */

		//Calls function to reduce the mesh
		MS.reduce(*(*(*manager.getModels())[currentModelID].getMesh()).getIndexedVertices(), *(*(*manager.getModels())[currentModelID].getMesh()).getIndices(), *(*(*manager.getModels())[currentModelID].getMesh()).getIndexedNormals(), continuousMeshSimplification);

		//Bind
		(*(*manager.getModels())[currentModelID].getMesh()).rebind();
		//End Bind

	}
	else if (((currentTime2 >= lastTime2 + 0.5) && (glfwGetKey(g_pWindow, GLFW_KEY_F11) == GLFW_PRESS) && continuousMeshSimplification == 0) || continuousMeshSimplification == -1) {
		lastTime2 = glfwGetTime();
		MS.reconstruct(*(*(*manager.getModels())[currentModelID].getMesh()).getIndexedVertices(), *(*(*manager.getModels())[currentModelID].getMesh()).getIndices(), continuousMeshSimplification);

		//Bind
		(*(*manager.getModels())[currentModelID].getMesh()).rebind();

	}
}

void drawModeInput() {
	//Set the draw mode. L to lines, P to points and F to fill.
	if (glfwGetKey(g_pWindow, GLFW_KEY_L) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if ((glfwGetKey(g_pWindow, GLFW_KEY_P) == GLFW_PRESS))
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	else if ((glfwGetKey(g_pWindow, GLFW_KEY_F) == GLFW_PRESS))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void transformationInput(long double& currentTime, long double& lastTime3, ModelManager& manager) {
	//Translação ao pressionar T
	if (glfwGetKey(g_pWindow, GLFW_KEY_T) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID < numModelos) {	//Translation ('T') - both
		lastTime3 = glfwGetTime();
		if (m_currentActive == MODEL) {
			(*manager.getModels())[currentModelID].addCompTransformation(&t, NULL, NULL, NULL, NULL, t.time);
			std::cout << "Queue size:" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (m_currentActive == CAMERA) {
			(*manager.getCameras())[currentCameraID].addCompTransformation(&t, NULL, NULL);
			std::cout << "Camera " << currentCameraID << " Queue size:" << (*(*manager.getCameras())[currentCameraID].getTransformationQueue()).size() << std::endl;
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_S) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID < numModelos) {	//Scale ('S')
		lastTime3 = glfwGetTime();
		(*manager.getModels())[currentModelID].addCompTransformation(NULL, NULL, &s, NULL, NULL, s.time);
		std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_R) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID < numModelos) {	//Rotation ('R') - both
		lastTime3 = glfwGetTime();
		if (m_currentActive == MODEL) {
			(*manager.getModels())[currentModelID].addCompTransformation(NULL, &r, NULL, NULL, NULL, r.time);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (m_currentActive == CAMERA) {
			(*manager.getCameras())[currentCameraID].addCompTransformation(NULL, &r, NULL);
			std::cout << "Camera " << currentCameraID << " Queue size:" << (*(*manager.getCameras())[currentCameraID].getTransformationQueue()).size() << std::endl;
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_A) == GLFW_PRESS && (currentTime > lastTime3 + 0.1) && currentModelID < numModelos) {	//Rotation Around point ('A') - both - buggy
		lastTime3 = glfwGetTime();
		if (m_currentActive == MODEL) {
			(*manager.getModels())[currentModelID].rotationAroundPoint(&p);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (m_currentActive == CAMERA) {
			(*manager.getCameras())[currentCameraID].aroundPoint(&cp);
			std::cout << "Camera " << currentCameraID << " Queue size:" << (*(*manager.getCameras())[currentCameraID].getTransformationQueue()).size() << std::endl;
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_C) == GLFW_PRESS && (currentTime > lastTime3 + 0.1) && currentModelID < numModelos) {	//Rotação + Translação + Escala ('C')
		lastTime3 = glfwGetTime();
		(*manager.getModels())[currentModelID].addCompTransformation(&t, &r, &s, NULL, NULL, compositionTime);
		std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_H) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID < numModelos) {	//Shear ('H')
		lastTime3 = glfwGetTime();
		(*manager.getModels())[currentModelID].addCompTransformation(NULL, NULL, NULL, &h, NULL, h.time);
		std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_Z) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID < numModelos) {	//Bezier ('Z') - both
		lastTime3 = glfwGetTime();
		if (m_currentActive == MODEL) {
			(*manager.getModels())[currentModelID].bezierCurve(b);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (m_currentActive == CAMERA) {
			(*manager.getCameras())[currentCameraID].bezierCurve(b);
			std::cout << "Queue size of camera" << currentCameraID << ":" << (*(*manager.getCameras())[currentCameraID].getTransformationQueue()).size() << std::endl;
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_B) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID < numModelos) {	//B-Spline ('B') - both
		lastTime3 = glfwGetTime();
		if (m_currentActive == MODEL) {
			(*manager.getModels())[currentModelID].BSplineTest(l);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (m_currentActive == CAMERA) {
			//(*manager.getCameras())[currentCameraID].BSpline(l);
			(*manager.getCameras())[currentCameraID].BSplineTest(l);
			std::cout << "Queue size of camera" << currentCameraID << ":" << (*(*manager.getCameras())[currentCameraID].getTransformationQueue()).size() << std::endl;
		}
	}

	else if (glfwGetKey(g_pWindow, GLFW_KEY_D) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID < numModelos) {	//Projection 3D ('D')
		lastTime3 = glfwGetTime();
		(*manager.getModels())[currentModelID].addCompTransformation(NULL, NULL, NULL, NULL, &p3D, 0);
		std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_K) == GLFW_PRESS && (currentTime > lastTime3 + 0.3)) {					//Look At (K) - Camera only
		lastTime3 = glfwGetTime();
		if (m_currentActive == CAMERA) {
			(*manager.getCameras())[currentCameraID].addCompTransformation(NULL, NULL, &lk);
			std::cout << "Camera " << currentCameraID << " Queue size:" << (*(*manager.getCameras())[currentCameraID].getTransformationQueue()).size() << std::endl;
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_INSERT) == GLFW_PRESS && (currentTime > lastTime3 + 0.3)) {	//Insere novo modelo ('Insert')
		lastTime3 = glfwGetTime();
		if (m_currentActive == MODEL) {
			if (m_currentMesh == SUZANNE)
				manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(0), newModelPos);
			else if (m_currentMesh == GOOSE)
				manager.createModel("mesh/goose.dds", "myTextureSampler", (*manager.getMeshes()).at(1), newModelPos);
			else if (m_currentMesh == CUBE)
				manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(2), newModelPos);
			numModelos++;
		}
		else if (m_currentActive == CAMERA) {
			//manager.createCamera(( getViewMatrix() * glm::translate(glm::mat4(1.0f), newCamPos * glm::vec3(-1)) ),getProjectionMatrix());
			manager.createCamera(fieldOfView, hAux / vAux, near, far, newCamPos, newCamUp, newCamSight);
			numCameras++;
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_F1) == GLFW_PRESS && (currentTime > lastTime3 + 0.3)) {	//Insert point to Linear Path (F1) - Camera only
		lastTime3 = glfwGetTime();
		if (m_currentActive == CAMERA) {
			linearPoints.push_back(linearPoint);
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_F2) == GLFW_PRESS && (currentTime > lastTime3 + 0.3 && linearPoints.size()>0)) {	//Put current Linear Path (F2) - Camera only
		lastTime3 = glfwGetTime();
		if (m_currentActive == CAMERA) {
			glm::vec3 previous, current;
			struct translation t;
			t.time = linearTime / linearPoints.size();
			previous = glm::vec3((*manager.getCameras())[currentCameraID].getViewMatrix()[3][0],
				(*manager.getCameras())[currentCameraID].getViewMatrix()[3][1],
				(*manager.getCameras())[currentCameraID].getViewMatrix()[3][2]);
			for (int i = 0; i < linearPoints.size(); i++) {
				current = linearPoints[i];
				t.translationVec = current - previous;

				(*manager.getCameras())[currentCameraID].addCompTransformation(&t, NULL, NULL);

				previous = current;
			}
			std::cout << "Camera " << currentCameraID << " Queue size:" << (*(*manager.getCameras())[currentCameraID].getTransformationQueue()).size() << std::endl;
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_DELETE) == GLFW_PRESS) {	//Resets all  input data
		t.translationVec = glm::vec3(0, 0, 0);
		t.time = 1.0f;
		s.scaleVec = glm::vec3(1);
		s.time = 1.0f;
		r.rotationVec = glm::vec3(1, 0, 0);
		r.rotationDegrees = 0.0f;
		r.time = 1.0f;
		h.shearVec = glm::vec3(0, 0, 0);
		h.time = 1.0f;
		b.controlPoints[0] = glm::vec3(0);
		b.controlPoints[1] = glm::vec3(0);
		b.controlPoints[2] = glm::vec3(0);
		b.time = 1.0f;
		l.controlPoints[0] = glm::vec3(0);
		l.controlPoints[1] = glm::vec3(0);
		l.controlPoints[2] = glm::vec3(0);
		l.controlPoints[3] = glm::vec3(0);
		l.time = 1.0f;
		p.rotationAngle = 0.0f;
		p.point = glm::vec3(0);
		p.time = 1.0f;
		p3D.projVector = glm::vec3(0, 0, 2);
		p3D.time = 1.0f;
		newModelPos = glm::vec3(0);
		newCamPos = glm::vec3(0);
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_U) == GLFW_PRESS && (currentTime > lastTime3 + 0.1)) {	//Apply all
		lastTime3 = glfwGetTime();
		if (m_currentActive == MODEL) {
			manager.setModelTransformation(currentModelID);
		}
		else if (m_currentActive == CAMERA) {
			manager.setCamerasTransformation(currentCameraID);
			//(*manager.getCameras())[currentCameraID].applyTransformation();
		}
	}
}

void shaderInput(long double& currentTime, long double& lastTime3, ModelManager& manager) {
	//Explosion Geometry Shader
	if (glfwGetKey(g_pWindow, GLFW_KEY_F5) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentShaderProgramID == 1) {
		if (m_currentActive == MODEL) {
			(*manager.getModels())[currentModelID].setGeometry(true);
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_F6) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentShaderProgramID == 1) {
		if (m_currentActive == MODEL)
			(*manager.getModels())[currentModelID].setGeometry(false);
	}
	else if (currentShaderProgramID != 1)
		(*manager.getModels())[currentModelID].setGeometry(false);
}


void WindowSizeCallBack(GLFWwindow *pWindow, int nWidth, int nHeight) {

	g_nWidth = nWidth;
	g_nHeight = nHeight;
	glViewport(0, 0, g_nWidth, g_nHeight);
	TwWindowSize(g_nWidth, g_nHeight);
}

void TW_CALL transformModel(void *, ModelManager manager, int model)
{
	manager.setModelTransformation(model);
}

int main(void)
{

	initialiseTransformationVariables();

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
	TwAddSeparator(g_pToolBar, "Mesh", NULL);
	TwAddButton(g_pToolBar, "Mesh options", NULL, NULL, "");
	
	// Array of drop down items
	TwEnumVal Meshes[] = { { SUZANNE, "Suzanne" },{ CUBE, "Cube" },{ GOOSE, "Goose" } };

	// ATB identifier for the array
	TwType MeshTwType = TwDefineEnum("MeshType", Meshes, 3);

	// Link it to the tweak bar
	TwAddVarRW(g_pToolBar, "Mesh: ", MeshTwType, &m_currentMesh, NULL);
	//New Model position
	
	TwAddVarRW(g_pToolBar, "New Model position:", TW_TYPE_DIR3D, &newModelPos, NULL);
	
	TwAddVarRW(g_pToolBar, "Active model: ", TW_TYPE_INT8, &currentModelID, "min=0 max=20 step=1 label='Active model");

	
	//Add Model-Camera Switch
		TwAddSeparator(g_pToolBar, "Model-Camera Switch:", NULL);

		// Array of drop down items
		TwEnumVal Actives[] = { { MODEL, "Model" },{ CAMERA, "Camera" } };

		// ATB identifier for the array
		TwType ActiveTwType = TwDefineEnum("ActiveType", Actives, 2);

		// Link it to the tweak bar
		TwAddVarRW(g_pToolBar, "Model-Camera Switch: ", ActiveTwType, &m_currentActive, NULL);

	//Add 'Shader' Options
	TwAddSeparator(g_pToolBar, "Shader Program", NULL);
	TwAddButton(g_pToolBar, "Shader options:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Active Shader Program: ", TW_TYPE_INT8, &currentShaderProgramID, "min=0 step = 1 label='Active Shader Program'");

	//Add 'Camera' Options
	TwAddSeparator(g_pToolBar, "Camera", NULL);
	TwAddButton(g_pToolBar, "Camera options:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Active Camera: ", TW_TYPE_INT8, &currentCameraID, "min=0 step = 1 label='Active Camera'");
	
	TwAddButton(g_pToolBar, "New Camera parameters:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "New Camera Position: ", TW_TYPE_DIR3F, &newCamPos, NULL);
	TwAddVarRW(g_pToolBar, "New Camera Up: ", TW_TYPE_DIR3F, &newCamUp, NULL);
	TwAddVarRW(g_pToolBar, "New Camera Sight: ", TW_TYPE_DIR3F, &newCamSight, NULL);

	TwAddVarRW(g_pToolBar, "Field Of View:", TW_TYPE_FLOAT, &fieldOfView, " min=0.0 step=0.1 label=''");
	TwAddVarRW(g_pToolBar, "Aspect Ratio Horizontal:", TW_TYPE_INT8, &hAux, " min=0.0 step=1 label=''");
	TwAddVarRW(g_pToolBar, "Aspect Ratio Vertical:", TW_TYPE_INT8, &vAux, " min=0.0 step=1 label=''");
	TwAddVarRW(g_pToolBar, "Near Plane:", TW_TYPE_FLOAT, &near, " min=0.0 step=0.1 label=''");
	TwAddVarRW(g_pToolBar, "Far Plane:", TW_TYPE_FLOAT, &far, " min=0.0 step=0.1 label=''");


	
	
	//Add 'Translation' options
	TwAddSeparator(g_pToolBar, "Translation", NULL);
	TwAddButton(g_pToolBar, "Translation (Both):", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Translation: ", TW_TYPE_DIR3F, &t.translationVec, " label='Translation to put to queue:");
	TwAddVarRW(g_pToolBar, "Translation Time: ", TW_TYPE_DOUBLE, &t.time, " min=0.0 step=0.1 label='Time to do the translation (seconds):");
	
	//Add 'LookAt' options
	TwAddSeparator(g_pToolBar, "Look At: ", NULL);
	TwAddButton(g_pToolBar, "Look At (Camera):", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Eye: ", TW_TYPE_DIR3F, &lk.eye, " label='eye position");
	TwAddVarRW(g_pToolBar, "Center: ", TW_TYPE_DIR3F, &lk.center, " label='point to look:");
	TwAddVarRW(g_pToolBar, "Up: ", TW_TYPE_DIR3F, &lk.up, " label='up vector:");
	TwAddVarRW(g_pToolBar, "Look At Time:", TW_TYPE_DOUBLE, &lk.time, " min=0.0 step=0.1 label='animation time'");

	//Add 'Rotation' options
	TwAddSeparator(g_pToolBar, "Rotation", NULL);
	TwAddButton(g_pToolBar, "Rotation parameters:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Rotation direction: ", TW_TYPE_DIR3F, &r.rotationVec, " label='Translation to put to queue:");
	TwAddVarRW(g_pToolBar, "Rotation Degrees:", TW_TYPE_FLOAT, &r.rotationDegrees, "step=0.5 label='Rotation angle'");
	TwAddVarRW(g_pToolBar, "Rotation Time:", TW_TYPE_DOUBLE, &r.time, " min=0.0 step=0.1 label='Rotation time'");

	//Add 'Rotation Around Point' options
	TwAddSeparator(g_pToolBar, "Rotation Around Point Camera", NULL);
	TwAddButton(g_pToolBar, "Rotation Around Point Camera: ", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Point:  ", TW_TYPE_DIR3F, &cp.point, "step=0.1");
	TwAddVarRW(g_pToolBar, "Orbit degrees:  ", TW_TYPE_FLOAT, &cp.rotationAngle, "step=0.5 label='Rotation angle'");
	TwAddVarRW(g_pToolBar, "Rotation Axis:  ", TW_TYPE_DIR3F, &cp.rotationAxis, "step=0.1");
	TwAddVarRW(g_pToolBar, "Rotation around point time:  ", TW_TYPE_DOUBLE, &cp.time, " min=0.0 step=0.1 label='Rotation around point time'");

	//Linear Path Options
	TwAddSeparator(g_pToolBar, "Linear Path Options", NULL);
	TwAddButton(g_pToolBar, "Linear Path: (P0 is current position)", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Number of points (automatic):  ", TW_TYPE_INT32, &numLinearPoints, " min=1.0 label=''");
	TwAddVarRW(g_pToolBar, "Point (F1 to add a point):  ", TW_TYPE_DIR3F, &linearPoint, "step=0.1");
	TwAddVarRW(g_pToolBar, "Linear Path Time (F2 to go trought path): ", TW_TYPE_DOUBLE, &linearTime, " min=0.0 step=0.1 label=''");


	//Add 'Scale' options
	TwAddSeparator(g_pToolBar, "'Scale'", NULL);
	TwAddButton(g_pToolBar, "Scale parameters:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Scale: ", TW_TYPE_DIR3F, &s.scaleVec, "min=0.1 step=0.1 label='Scale to put to queue:");
	TwAddVarRW(g_pToolBar, "Scaling Time: ", TW_TYPE_DOUBLE, &s.time, " min=0.0 step=0.1 help='Time to do the scaling (seconds)' ");

	//Add 'Composition animation' options
	
	TwAddSeparator(g_pToolBar, "Composition", NULL);
	TwAddButton(g_pToolBar, "Compasition parameters:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Composition Time:", TW_TYPE_DOUBLE, &compositionTime, " min=0.0 step=0.1 label='Composition time'");

	//Add 'Rotation around point' options
	TwAddSeparator(g_pToolBar, "Rotation around point", NULL);
	TwAddButton(g_pToolBar, "Rotation around point parameters:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Point to be rotated around: ", TW_TYPE_DIR3F, &p.point, "step=0.1");
	TwAddVarRW(g_pToolBar, "Orbit degrees:", TW_TYPE_FLOAT, &p.rotationAngle, "step=0.5 label='Rotation angle'");
	TwAddVarRW(g_pToolBar, "Rotation around point time:", TW_TYPE_DOUBLE, &p.time, " min=0.0 step=0.1 label='Rotation around point time'");

	TwAddSeparator(g_pToolBar, "Projection 3D", NULL);
	TwAddButton(g_pToolBar, "Projection 3D:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Projection Vector: ", TW_TYPE_DIR3F, &p3D.projVector, "step=0.1");
	//TwAddVarRW(g_pToolBar, "Projection Time", TW_TYPE_DOUBLE, &p3D.time, " min=0.0 step=0.1");

	//Add 'Shear' Options
	TwAddSeparator(g_pToolBar, "Shear", NULL);
	TwAddButton(g_pToolBar, "Shear:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Shear vector: ", TW_TYPE_DIR3F, &h.shearVec, "step=0.1");
	TwAddVarRW(g_pToolBar, "Shear time:", TW_TYPE_DOUBLE, &h.time, " min=0.0 step=0.1 label='Shear time'");

	//Add 'Bezier' Options
	TwAddSeparator(g_pToolBar, "Bezier curve", NULL);
	TwAddButton(g_pToolBar, "Bezier Curve:", NULL, NULL, "");
	TwAddButton(g_pToolBar, "P0: current position.", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "P1:", TW_TYPE_DIR3F, &b.controlPoints[1], "step=0.1");
	TwAddVarRW(g_pToolBar, "P2:", TW_TYPE_DIR3F, &b.controlPoints[2], "step=0.1");
	TwAddVarRW(g_pToolBar, "Bezier Time:", TW_TYPE_DOUBLE, &b.time, " min=0.0 step=0.1 label='Bezier time'");

	//Add 'B-Spline' Options
	TwAddSeparator(g_pToolBar, "B-Spline curve", NULL);
	TwAddButton(g_pToolBar, "B-Spline Curve:", NULL, NULL, "");
	TwAddButton(g_pToolBar, "P0: current position. ", NULL, NULL, "");
	//TwAddVarRW(g_pToolBar, "P0: ", TW_TYPE_DIR3F, &l.controlPoints[0], "step=0.1");
	TwAddVarRW(g_pToolBar, "P1: ", TW_TYPE_DIR3F, &l.controlPoints[1], "step=0.1");
	TwAddVarRW(g_pToolBar, "P2: ", TW_TYPE_DIR3F, &l.controlPoints[2], "step=0.1");
	TwAddVarRW(g_pToolBar, "P3: ", TW_TYPE_DIR3F, &l.controlPoints[3], "step=0.1");
	TwAddVarRW(g_pToolBar, "B-Spline Time:", TW_TYPE_DOUBLE, &l.time, " min=0.0 step=0.1 label='B-Spline time'");

	TwAddSeparator(g_pToolBar, "End", NULL);


	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(g_pWindow, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(g_pWindow, g_nWidth / 2, g_nHeight / 2);

	//Gray background
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	//Model Manager
	ModelManager manager = ModelManager();
	manager.createShader("shaders/PassThroughShading.vertexshader", "shaders/PassThroughShading.fragmentshader", "shaders/PassThroughGeometryShader.gs");
	manager.createShader("shaders/ExplosionGeometryShading.vertexshader", "shaders/ExplosionGeometryShading.fragmentshader", "shaders/ExplosionGeometryShader.gs");
	manager.createShader("shaders/VisualizeNormalsShading.vertexshader", "shaders/VisualizeNormalsShading.fragmentshader", "shaders/VisualizeNormalsGeometryShader.gs");
	manager.useShader(0);
	
	GLuint VertexArrayID = manager.getVertexArrayID();
		
	// Get a handle for our "MVP" uniform
	GLuint MatrixID      = glGetUniformLocation(manager.getProgramID(), "MVP");
	//GLuint ViewMatrixID  = glGetUniformLocation(manager.getProgramID(), "V");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(manager.getProgramID());
	GLuint LightID = glGetUniformLocation(manager.getProgramID(), "LightPosition_worldspace");

	//Speed/Times Auxiliaries
	long double lastTime = glfwGetTime(), lastTime2 = glfwGetTime();
	int nbFrames = 0;
	int simplify = 1;
	long double lastTime3 = glfwGetTime();
	

	// Get a handle for our "LightPosition" uniform
	glUseProgram(manager.getProgramID());
	LightID = glGetUniformLocation(manager.getProgramID(), "LightPosition_worldspace");

	//Print Instructions
	printInstructions();

	//----- Trabalho 1: Mesh Simplification
	int continuousMeshSimplification = 0;
	MeshSimplification MS;
	// Read our .obj file and creates meshes
	//manager.createMesh("mesh/suzanne.obj");
	//manager.createMesh("mesh/goose.obj");
	//manager.createMesh("mesh/cube.obj");
	//manager.createMesh("mesh/nanosuit.obj");
	
	manager.loadMeshes("mesh/nanosuit.obj");

	//----- Trabalho 2: Model Transformation
	//Creates models
	if ((*manager.getMeshes()).size() > 0)
		manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(0), glm::vec3(3, 3, 0));
	//manager.createModel("mesh/goose.dds", "myTextureSampler", (*manager.getMeshes()).at(1), glm::vec3(-3, 3, 0));
	//manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(2), glm::vec3(3, -3, 0));
	//manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(3), glm::vec3(-3, -3, 0));
	
	//----- Trabalho 3: Câmeras
	//Creates Camera 1
	computeMatricesFromInputs(nUseMouse, g_nWidth, g_nHeight);
	manager.createCamera(getViewMatrix() * glm::translate(glm::mat4(1.0f), glm::vec3(-1)*glm::vec3(0, 0, 10)), getProjectionMatrix());
	//Creates Camera 2
	manager.createCamera(getViewMatrix(), getProjectionMatrix());
	 
	//Trabalho 4: Shaders
	bool drawNormals = false;
	
	//Draw Loop
	do{
        check_gl_error();
		// Measure speed
		long double currentTime2 = glfwGetTime();
		
		//Control counter's limits
		controlCounterVariables(manager);

		//Draw Mode Input
		drawModeInput();

		/* ----- Trabalho 1: Mesh Simplification ----- */
		meshSimplificationInput(
			continuousMeshSimplification,
			currentTime2,
			lastTime2,
			MS,
			currentModelID,
			manager);

        //use the control key to free the mouse
		if (glfwGetKey(g_pWindow, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
			nUseMouse = 0;
		else
			nUseMouse = 1;
		long double currentTime = glfwGetTime();

		/* --- Trabalho 4 --- */
		shaderInput(currentTime, lastTime3, manager);

		/* --- Trabalho 2 --- */
		transformationInput(currentTime, lastTime3, manager);

		//Camera Noise
		manager.cameraNoise();
		//Transform cameras
		manager.transformCameras();
		//Transform models
		manager.transformModels();

		
		// Measure speed
		currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			if (glfwGetKey(g_pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
				// printf and reset
				printf("%f ms/frame\t Contious state: %d\n", 1000.0 / double(nbFrames), continuousMeshSimplification);
			}
			nbFrames = 0;
			lastTime += 1.0;
		}
		
		//Changes shader
		if (glfwGetKey(g_pWindow, GLFW_KEY_F12) == GLFW_PRESS) {
			manager.useShader(currentShaderProgramID);
			if (currentShaderProgramID == 2) {
				drawNormals = true;
				std::cout << "DEBUG::DRAWNORMALS==TRUE::DEBUG" << std::endl;
			}
			else {
				drawNormals = false;
				std::cout << "DEBUG::DRAWNORMALS==FALSE::DEBUG" << std::endl;
			}
		}

		manager.clearScreen();

		//Is Normal Shader Active?
		if (drawNormals == true) {
			manager.useShader(0);
			//Draw
			manager.drawModels((*manager.getCameras())[currentCameraID].getViewMatrixID(), (*manager.getCameras())[currentCameraID].getViewMatrix(), (*manager.getCameras())[currentCameraID].getProjectionMatrix(), g_pWindow);
			manager.useShader(2);
		}
		
		//Draw
		manager.drawModels((*manager.getCameras())[currentCameraID].getViewMatrixID(), (*manager.getCameras())[currentCameraID].getViewMatrix(), (*manager.getCameras())[currentCameraID].getProjectionMatrix(), g_pWindow);
		manager.swapBuffers(g_pWindow);

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(g_pWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(g_pWindow) == 0);

	// Cleanup VBO and shader
	manager.cleanup();

	// Terminate AntTweakBar and GLFW
	TwTerminate();
	glfwTerminate();

	return 0;
}

