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

//custom includes
#include "meshsimplification.hpp"
#include "model.hpp"

using namespace glm;

#include <shader.hpp>
#include <texture.hpp>
#include <controls.hpp>
#include <objloader.hpp>
#include <vboindexer.hpp>
#include <glerror.hpp>

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
	std::cout << "\t4. Movimentação:" << std::endl;
	std::cout << "\t\t3.1. A tecla \"Control\" libera o mouse." << std::endl;
	std::cout << "\t\t3.2. As setas direcionais podem ser usadas para movimentao." << std::endl;
	std::cout << "\t Ao manter-se a tecla \"Left Shift\" pressionada, sera impresso um print de performance a cada 1 segundo." << std::endl;
	std::cout << "-----------------------------------" << std::endl;
}

void WindowSizeCallBack(GLFWwindow *pWindow, int nWidth, int nHeight) {

	g_nWidth = nWidth;
	g_nHeight = nHeight;
	glViewport(0, 0, g_nWidth, g_nHeight);
	TwWindowSize(g_nWidth, g_nHeight);
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

	//Gray background
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
	GLuint programID = LoadShaders("shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader");

	// Read our .obj file and creates mesh
	Mesh suzanne("mesh/suzanne.obj");

	//Creates model
	Model model("mesh/uvmap.DDS", "myTextureSampler", programID, &suzanne);


	// Get a handle for our "MVP" uniform
	GLuint MatrixID      = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID  = glGetUniformLocation(programID, "V");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime(), lastTime2 = glfwGetTime();
	int nbFrames    = 0;
	int simplify = 1;
	
	

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	int continuousMeshSimplification = 0;
	MeshSimplification MS;

	printInstructions();


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
	
		if ((currentTime2 >= lastTime2 + 0.5  && (glfwGetKey(g_pWindow,GLFW_KEY_N) == GLFW_PRESS) && continuousMeshSimplification == 0) || continuousMeshSimplification == 1) {

			lastTime2 = glfwGetTime();
			/* ------------ Trabalho 1 ----------------- *
			* -- Algoritmo de simplificação de Mesh --- *
			* - Parte 1: Cálculo inicial de vizinhos ---*
			* - Parte 2: Remoção do Vértice ----------- *
			* - Parte 3: Collapse --------------------- *
			* - Parte 4: Cálculo dos vizinhos alterados *
			* ----------------------------------------- */

			//Calls function to reduce the mesh
			MS.reduce(*(*model.getMesh()).getIndexedVertices(), *(*model.getMesh()).getIndices(), *(*model.getMesh()).getIndexedNormals(), continuousMeshSimplification);

			//Bind
			(*model.getMesh()).rebind();

			// Get a handle for our "LightPosition" uniform
			glUseProgram(programID);
			LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

			//End Bind
		
		}
		else if (((currentTime2 >= lastTime2 + 0.5) && (glfwGetKey(g_pWindow, GLFW_KEY_B) == GLFW_PRESS) && continuousMeshSimplification == 0) || continuousMeshSimplification == -1 ) {
			lastTime2 = glfwGetTime();
			MS.reconstruct(*(*model.getMesh()).getIndexedVertices(), *(*model.getMesh()).getIndices(), continuousMeshSimplification);

			//Bind
			(*model.getMesh()).rebind();

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
			if (glfwGetKey(g_pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
				// printf and reset
				printf("%f ms/frame\t Contious state: %d\n", 1000.0 / double(nbFrames), continuousMeshSimplification);
			}
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
		glUniformMatrix4fv(model.getModelMatrixID(), 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *model.getTexture());
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(model.getTextureID(), 0);

		(*model.getMesh()).loadMesh();

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,        // mode
			(*(*model.getMesh()).getIndices()).size(),      // count
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
	(*model.getMesh()).cleanup();
	glDeleteProgram(programID);
	glDeleteTextures(1, model.getTexture());
	glDeleteVertexArrays(1, &VertexArrayID);

	// Terminate AntTweakBar and GLFW
	TwTerminate();
	glfwTerminate();

	return 0;
}

