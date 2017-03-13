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


void controlCounterVariables(ModelManager& manager) 
{

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

void drawModeInput() {
	//Set the draw mode. L to lines, P to points and F to fill.
	if (glfwGetKey(g_pWindow, GLFW_KEY_L) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if ((glfwGetKey(g_pWindow, GLFW_KEY_P) == GLFW_PRESS))
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	else if ((glfwGetKey(g_pWindow, GLFW_KEY_F) == GLFW_PRESS))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


/* ----- Spawns ---- */
//CameraSpawn
void cameraSpawn(ModelManager& manager) {
	/* -- Camera -- */
	computeMatricesFromInputs(nUseMouse, g_nWidth, g_nHeight);
	manager.createCamera(getViewMatrix() * glm::translate(glm::mat4(1.0f), glm::vec3(-1)*glm::vec3(0, 0, 20)), getProjectionMatrix());

}
//Enemies
void enemiesSpawn(ModelManager& manager) {
	std::vector <Mesh> meshes;
	manager.loadMeshes("mesh/suzanne.obj");
	meshes.clear();
	meshes = *manager.getMeshes();
	std::cout << "DEBUG:: LOADING ENEMIES MODELS... ::DEBUG" << std::endl;
	manager.createModel("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-3, 10,-5), "Enemy");
	manager.createModel("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(0, 10, -5), "Enemy");
	manager.createModel("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(3, 10, -5), "Enemy");
	std::cout << "DEBUG:: FINISHED LOADING ENEMIES MODELS ::DEBUG" << std::endl;
}
//Player
void playersSpawn(ModelManager& manager) {
	std::vector <Mesh> meshes;
	std::cout << "DEBUG:: LOADING PLAYER MODEL... ::DEBUG" << std::endl;
	manager.loadMeshes("mesh/goose.obj");
	meshes.clear();
	meshes.push_back((*manager.getMeshes())[1]);

	Model player("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(0, -10, -5), "Player");
	player.setModelMatrix(glm::rotate(player.getModelMatrix(), 180.0f, glm::vec3(0, 1, 0)));

	manager.addModel(player);
	std::cout << "DEBUG:: FINISHED LOADING PLAYER MODEL ::DEBUG" << std::endl;

}
/* ----------------- */

/* ------ Player Movement ----- */
void playerMovement(ModelManager& manager) {
	if (glfwGetKey(g_pWindow, GLFW_KEY_A) == GLFW_PRESS) {
		std::cout << "Player x (" << (*manager.getPlayers())[0].getPosition().x << ") > -15" << std::endl;
		if ((*manager.getPlayers())[0].getPosition().x > -15.0f) {
			t.time = 0.05;
			t.translationVec = glm::vec3(0.2, 0, 0);
			(*manager.getPlayers())[0].addCompTransformation(&t, NULL, NULL, NULL, NULL, t.time);
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_D) == GLFW_PRESS) {
		std::cout << "Player x (" << (*manager.getPlayers())[0].getPosition().x << ") < 15" << std::endl;
		if ((*manager.getPlayers())[0].getPosition().x < 15.0f) {
			t.time = 0.05;
			t.translationVec = glm::vec3(-0.2, 0, 0);
			(*manager.getPlayers())[0].addCompTransformation(&t, NULL, NULL, NULL, NULL, t.time);
		}
	}
}
/* ---------------------------- */

void WindowSizeCallBack(GLFWwindow *pWindow, int nWidth, int nHeight) {

	g_nWidth = nWidth;
	g_nHeight = nHeight;
	glViewport(0, 0, g_nWidth, g_nHeight);
	TwWindowSize(g_nWidth, g_nHeight);
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

	//Add 'Shader' Options
	TwAddSeparator(g_pToolBar, "Shader Program", NULL);
	TwAddButton(g_pToolBar, "Shader options:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Active Shader Program: ", TW_TYPE_INT8, &currentShaderProgramID, "min=0 step = 1 label='Active Shader Program'");

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
	//Shaders
	manager.createShader("shaders/PassThroughShading.vertexshader", "shaders/PassThroughShading.fragmentshader", "shaders/PassThroughGeometryShader.gs");
	manager.createShader("shaders/ExplosionGeometryShading.vertexshader", "shaders/ExplosionGeometryShading.fragmentshader", "shaders/ExplosionGeometryShader.gs");
	manager.createShader("shaders/VisualizeNormalsShading.vertexshader", "shaders/VisualizeNormalsShading.fragmentshader", "shaders/VisualizeNormalsGeometryShader.gs");
	manager.useShader(1);
	
	GLuint VertexArrayID = manager.getVertexArrayID();
		
	// Get a handle for our "MVP" uniform
	GLuint MatrixID      = glGetUniformLocation(manager.getProgramID(), "MVP");
	//GLuint ViewMatrixID  = glGetUniformLocation(manager.getProgramID(), "V");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(manager.getProgramID());
	GLuint LightID = glGetUniformLocation(manager.getProgramID(), "LightPosition_worldspace");

	//Speed/Times Auxiliaries
	long double lastTime = glfwGetTime();
	int nbFrames = 0;
	int simplify = 1;
	long double lastTime3 = glfwGetTime();
	long double collisionCheckTime = glfwGetTime();
	

	// Get a handle for our "LightPosition" uniform
	glUseProgram(manager.getProgramID());
	LightID = glGetUniformLocation(manager.getProgramID(), "LightPosition_worldspace");

	
	// Read our .obj file and creates meshes
	//manager.loadMeshes("mesh/cube.obj");
	//manager.loadMeshes("mesh/g1.obj");
	//manager.loadMeshes("mesh/luxury_house.obj");
	//manager.loadMeshes("mesh/nanosuit.obj");


	//Spawn
	cameraSpawn(manager);
	enemiesSpawn(manager);
	playersSpawn(manager);

	// ------ Shader Control
	bool drawNormals = false;
	
	// ------- Transformation Control
	manager.setTransformEnemies(true);
	long double lastPlayerMovement = glfwGetTime();

	//Draw Loop
	do{
        check_gl_error();
		// Measure speed
		long double currentTime = glfwGetTime();
		
		//Control counter's limits
		controlCounterVariables(manager);

		//Draw Mode Input
		drawModeInput();

        //use the control key to free the mouse
		if (glfwGetKey(g_pWindow, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
			nUseMouse = 0;
		else
			nUseMouse = 1;
		

		/* --- Shaders --- */
		//Explode
		//(*manager.getModels())[currentModelID].setGeometry(true);

		//Stop Explosion
		//(*manager.getModels())[currentModelID].setGeometry(false);

		/* --- Transformações --- */
		//Translação		
		//(*manager.getEnemies())[0].addCompTransformation(&t, NULL, NULL, NULL, NULL, t.time);

		//Apply
		//manager.setModelTransformation(currentModelID);

		//Collisions
		if ((currentTime > lastTime3 + 5)) {
			lastTime3 = glfwGetTime();
			manager.printCollisions();
		}
		if (currentTime > collisionCheckTime + 0.1) {
			collisionCheckTime = glfwGetTime();
			manager.checkAllModelsCollision();
		}

		//Player Input
		if (currentTime > lastPlayerMovement + 0.05f) {
			playerMovement(manager);
			lastPlayerMovement = glfwGetTime();
		}

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
				printf("%f ms/frame\n", 1000.0 / double(nbFrames));
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

		//Is Normals Shader Active?
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

