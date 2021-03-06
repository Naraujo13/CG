/* ---------------- Author: N�colas Oreques de Araujo ---------------- */

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

//Defines
#define ENEMY_BASE_SPEED 0.006125f * 30

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
//N�mero de elementos
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
	manager.loadMeshes("mesh/Space_Invader.obj");
	meshes.clear();
	meshes = *manager.getMeshes();
	std::cout << "DEBUG:: LOADING ENEMIES MODELS... ::DEBUG" << std::endl;
	for (int i = 10; i >= 1; i = i - 3) {
		for (int j = -12; j <= 12; j = j + 3) { //-12 a 12
			Enemy enemy("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(j, i, -5), "Enemy", ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
			enemy.setModelMatrix(glm::scale(enemy.getModelMatrix(), glm::vec3(0.01f)));
			manager.addModel(enemy);
		}
	}
	//Suzanne 
/*
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-9, 10, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-6, 10, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-3, 10,-5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(0, 10, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(3, 10, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(6, 10, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(9, 10, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-9, 7, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-6, 7, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-3, 7, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(0, 7, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(3, 7, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(6, 7, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(9, 7, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-9, 4, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-6, 4, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(-3, 4, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(0, 4, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(3, 4, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), manager.getDifficulty());
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(6, 4, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), 1);
	manager.createEnemy("mesh/uvmap.DDS", "myTextureSampler", meshes, glm::vec3(9, 4, -5), ENEMY_BASE_SPEED * manager.getDifficulty(), 1);
	std::cout << "DEBUG:: FINISHED LOADING ENEMIES MODELS ::DEBUG" << std::endl;*/
}
//Player
void playersSpawn(ModelManager& manager) {
	std::vector <Mesh> meshes;
	std::cout << "DEBUG:: LOADING PLAYER MODEL... ::DEBUG" << std::endl;
	manager.loadMeshes("mesh/sphere.obj");
	meshes.clear();
	meshes.push_back((*manager.getMeshes())[60]);

	Model player("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(0, -10, -5), "Player");
	player.setModelMatrix(glm::rotate(player.getModelMatrix(), 180.0f, glm::vec3(0, 1, 0)));

	manager.addModel(player);
	std::cout << "DEBUG:: FINISHED LOADING PLAYER MODEL ::DEBUG" << std::endl;
	
	manager.loadMeshes("mesh/capsule.obj"); //[61]

}
//Scenerie
void scenerieSpawn(ModelManager& manager) {
	std::vector <Mesh> meshes;
	std::cout << "DEBUG:: LOADING SCENERIE MODEL... ::DEBUG" << std::endl;
	manager.loadMeshes("mesh/sphere.obj"); //[62]
	meshes.clear();
	meshes.push_back((*manager.getMeshes())[62]);
	
	Model scenerie("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(-5 , 5 , -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.45)));
	manager.addModel(scenerie);
	
	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(3, 12, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.35)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(12, 7, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.6)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(-11, 12, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.24)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(6, -10, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.78)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(0.5, -3, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.56)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(-6.89, -8.98, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.256)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(-12.35, 1, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.385)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(-9.56, -14, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.67)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(14, -1.5, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.466)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(7.5, 7.5, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.3256)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(15.7, -13.5, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.355)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(-15.7, -8.5, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.4896)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(-14.7, 10.3, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.3867)));
	manager.addModel(scenerie);

	scenerie = Model("mesh/goose.dds", "myTextureSampler", manager.getProgramID(), meshes, glm::vec3(12.662, -12.5, -10), "Scenerie");
	scenerie.setModelMatrix(glm::scale(scenerie.getModelMatrix(), glm::vec3(0.192)));
	manager.addModel(scenerie);


	std::cout << "DEBUG:: FINISHED LOADING PLAYER MODEL ::DEBUG" << std::endl;

	
}
/* ----------------- */

/* ------ Player Input ----- */
void playerMovement(ModelManager& manager) {
	if (glfwGetKey(g_pWindow, GLFW_KEY_A) == GLFW_PRESS) {
		if ((*manager.getPlayers())[0].getPosition().x > -15.0f) {
			t.time = 0.05;
			t.translationVec = glm::vec3(0.2, 0, 0);
			(*manager.getPlayers())[0].addCompTransformation(&t, NULL, NULL, NULL, NULL, t.time);
		}
	}
	else if (glfwGetKey(g_pWindow, GLFW_KEY_D) == GLFW_PRESS) {
		if ((*manager.getPlayers())[0].getPosition().x < 15.0f) {
			t.time = 0.05;
			t.translationVec = glm::vec3(-0.2, 0, 0);
			(*manager.getPlayers())[0].addCompTransformation(&t, NULL, NULL, NULL, NULL, t.time);
		}
	}
}

void playerShooting(ModelManager& manager) {
	if (glfwGetKey(g_pWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
		std::vector <Mesh> meshes;
		//std::cout << "DEBUG:: LOADING PROJECTILE MODEL... ::DEBUG" << std::endl;

		meshes.push_back((*manager.getMeshes())[61]);
		Projectile projectile("mesh/uvmap.dds", "myTextureSampler", manager.getProgramID(), meshes, (*manager.getPlayers())[0].getPosition(), "Projectile", manager.getDifficulty()*0.025f);
		projectile.setModelMatrix(glm::translate(projectile.getModelMatrix(), glm::vec3(0,2.0f,0)));
		projectile.setModelMatrix(glm::rotate(projectile.getModelMatrix(), 90.0f, glm::vec3(1, 0, 0)));
		projectile.setModelMatrix(glm::scale(projectile.getModelMatrix(), glm::vec3(0.25f,0.25f,0.25f)));

		manager.addModel(projectile);
		//std::cout << "DEBUG:: FINISHED LOADING PROJECTILE MODEL ::DEBUG" << std::endl;
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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

	//Spawn
	cameraSpawn(manager);
	enemiesSpawn(manager);
	playersSpawn(manager);
	//scenerieSpawn(manager);


	// ------ Shader Control
	bool drawNormals = false;
	
	// ------- Transformation Control
	manager.setTransformEnemies(true);
	long double lastPlayerMovement = glfwGetTime();
	long double lastPlayerShooting = glfwGetTime();

	// ------- Dead Models Collector
	long double lastCollected = glfwGetTime();

	//Draw Loop
	do{
		if ((*manager.getEnemies()).empty()) {
			manager.increaseDifficulty();
			enemiesSpawn(manager);
		}
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

		//Dead Models Collector
		if (currentTime > lastCollected + 0.5f) {
			lastCollected = glfwGetTime();
			manager.deadModelsCollector();
		}

		//Collisions
		if ((currentTime > lastTime3 + 5)) {
			lastTime3 = glfwGetTime();
			//manager.printCollisions();
		}
		if (currentTime > collisionCheckTime + 0.1) {
			collisionCheckTime = glfwGetTime();
			manager.checkAllModelsCollision();
		}

		//Player Input
		//Movement
		if (currentTime > lastPlayerMovement + 0.05f) {
			playerMovement(manager);
			lastPlayerMovement = glfwGetTime();
		}
		//Shooting
		if (currentTime > lastPlayerShooting + 0.3f) {
			playerShooting(manager);
			lastPlayerShooting = glfwGetTime();
		}

		//Projectiles Movement
		manager.projectilesMovementPattern();

		//Enemies Movement
		manager.movesEnemyInPattern();

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

