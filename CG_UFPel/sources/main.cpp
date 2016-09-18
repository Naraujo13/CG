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
	std::cout << "\t\t3.1. A tecla \"Control\" libera o mouse." << std::endl;
	std::cout << "\t\t3.2. As setas direcionais podem ser usadas para movimentao." << std::endl;
	std::cout << "\t5. Controle Geral:" << std::endl;
	std::cout << "\t\t5.1.O modelo ativo no momento pode ser escolhido atraves do campo da interface ou das teclas numericas." << std::endl;
	std::cout << "\t\t5.2. A tecla \"Delete\" ira resetar todas as entradas da interface." << std::endl;
	std::cout << "\t6. Transformacoes sobre modelos:" << std::endl;
	std::cout << "\t\t6.1. A tecla \"T\" adiciona uma translacao com os parametros selecionados na interface." << std::endl;
	std::cout << "\t\t6.2. A tecla \"S\" adiciona uma escala com os parametros selecionados na interface." << std::endl;
	std::cout << "\t\t6.3. A tecla \"R\" adiciona uma rotacao com os parametros selecionados na interface." << std::endl;
	std::cout << "\t\t6.4. A tecla \"I\" adiciona uma transformacao composta usando os parametros de rotacao e translacao, e o tempo pra realizar a animacao composta." << std::endl;
	std::cout << "\t\t6.5. A tecla \"H\" adiciona uma transformacao de shear com os parametros selecionados na interface" << std::endl;
	std::cout << "\t\t6.6. A tecla \"U\" ativa a realizacao de todas as transformacoes na fila." << std::endl;
	std::cout << "\t\t Todos os comandos da secao 6 serao realizados para o modelo ativo no momento." << std::endl;
	std::cout << "\t Ao manter-se a tecla \"Left Shift\" pressionada, sera impresso um print de performance a cada 1 segundo." << std::endl;
	std::cout << "----------------------------------" << std::endl;
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
	int currentModelID = 0;
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
	TwAddSeparator(g_pToolBar, "Mesh", NULL);
	TwAddButton(g_pToolBar, "Mesh options", NULL, NULL, "");
	//Meshes
	// Create an internal enum to name the meshes
	typedef enum { SUZANNE, CUBE, GOOSE } MESH_TYPE;

	// A variable for the current selection - will be updated by ATB
	MESH_TYPE m_currentMesh = SUZANNE;

	// Array of drop down items
	TwEnumVal Meshes[] = { { SUZANNE, "Suzanne" },{ CUBE, "Cube" },{ GOOSE, "Goose" } };

	// ATB identifier for the array
	TwType MeshTwType = TwDefineEnum("MeshType", Meshes, 3);

	// Link it to the tweak bar
	TwAddVarRW(g_pToolBar, "Mesh", MeshTwType, &m_currentMesh, NULL);
	TwAddVarRW(g_pToolBar, "Active model: ", TW_TYPE_INT8, &currentModelID, "min=0 max=3 step=1 label='Active model");


	//Add 'Translation' options
	TwAddSeparator(g_pToolBar, "Translation", NULL);
	TwAddButton(g_pToolBar, "Translation parameters:", NULL, NULL, "");
	glm::vec3 translationVector(0,0,0);
	double translationTime=1.0;
	TwAddVarRW(g_pToolBar, "Translation: ", TW_TYPE_DIR3F, &translationVector, " label='Translation to put to queue:");
	TwAddVarRW(g_pToolBar, "Translation Time: ", TW_TYPE_DOUBLE, &translationTime, " min=0.1 step=0.1 label='Time to do the translation (seconds):");
	

	//Add 'Scale' options
	TwAddSeparator(g_pToolBar, "'Scale'", NULL);
	TwAddButton(g_pToolBar, "Scale parameters:", NULL, NULL, "");
	double scaleVector = 1.0;
	double scaleTime = 1.0;
	TwAddVarRW(g_pToolBar, "Scale: ", TW_TYPE_DOUBLE, &scaleVector, "min=0.1 step=0.1 label='Scale to put to queue:");
	TwAddVarRW(g_pToolBar, "Scaling Time: ", TW_TYPE_DOUBLE, &scaleTime, " min=0.1 step=0.1 help='Time to do the scaling (seconds)' ");

	//Add 'Rotation' options
	TwAddSeparator(g_pToolBar, "Rotation", NULL);
	TwAddButton(g_pToolBar, "Rotation parameters:", NULL, NULL, "");
	//quat rotationQuat;
	float rotationDegrees = 0.0f;
	double rotationTime = 1.0f;
	glm::vec3 rotationDirection(1, 0, 0);
	//TwAddVarRW(g_pToolBar, "ObjRotation", TW_TYPE_QUAT4F, &rotationQuat, " axisz=-z ");
	TwAddVarRW(g_pToolBar, "Rotation direction: ", TW_TYPE_DIR3F, &rotationDirection, " label='Translation to put to queue:");
	TwAddVarRW(g_pToolBar, "Rotation Degrees:", TW_TYPE_FLOAT, &rotationDegrees, " min=0.0 step=0.5 label='Rotation angle'");
	TwAddVarRW(g_pToolBar, "Rotation Time:", TW_TYPE_DOUBLE, &rotationTime, " min=0.1 step=0.1 label='Rotation time'");
	
	//Add 'Composition animation' options
	double compositionTime = 0.0f;
	TwAddSeparator(g_pToolBar, "Composition", NULL);
	TwAddButton(g_pToolBar, "Compasition parameters:", NULL, NULL, "");
	TwAddVarRW(g_pToolBar, "Composition Time:", TW_TYPE_DOUBLE, &compositionTime, " min=0.1 step=0.1 label='Composition time'");

	//Add 'Rotation around point' options
	TwAddSeparator(g_pToolBar, "Rotation around point", NULL);
	TwAddButton(g_pToolBar, "Rotation around point parameters:", NULL, NULL, "");
	glm::vec3 rotationAP(1, 0, 0);
	double rotationAPTime = 1.0f;
	TwAddVarRW(g_pToolBar, "Point to be rotated: ", TW_TYPE_DIR3F, &rotationAP, "step=0.1");
	TwAddVarRW(g_pToolBar, "Rotation around point time:", TW_TYPE_DOUBLE, &rotationAPTime, " min=0.1 step=0.1 label='Rotation around point time'");

	//Add 'Shear' Options
	TwAddSeparator(g_pToolBar, "Shear", NULL);
	TwAddButton(g_pToolBar, "Shear:", NULL, NULL, "");
	glm::vec3 shearVector(0, 0, 0);
	double shearTime = 1.0f;
	TwAddVarRW(g_pToolBar, "Shear vector: ", TW_TYPE_DIR3F, &shearVector, "step=0.1");
	TwAddVarRW(g_pToolBar, "Shear time:", TW_TYPE_DOUBLE, &shearTime, " min=0.0 step=0.1 label='Shear time'");

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


	// Create and compile our GLSL program from the shaders

	//Model Manager
	ModelManager manager("shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader");
	
	GLuint VertexArrayID = manager.getVertexArrayID();

	// Read our .obj file and creates meshes
	manager.createMesh("mesh/suzanne.obj");
	manager.createMesh("mesh/goose.obj");
	manager.createMesh("mesh/cube.obj");
	
	//Creates models
	manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(0), glm::vec3(3,3,0));
	manager.createModel("mesh/goose.dds", "myTextureSampler", (*manager.getMeshes()).at(1), glm::vec3(-3,3,0));
	manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(0), glm::vec3(3,-3,0));
	manager.createModel("mesh/uvmap.DDS", "myTextureSampler", (*manager.getMeshes()).at(2), glm::vec3(-3,-3,0));

	// Get a handle for our "MVP" uniform
	GLuint MatrixID      = glGetUniformLocation(manager.getProgramID(), "MVP");
	GLuint ViewMatrixID  = glGetUniformLocation(manager.getProgramID(), "V");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(manager.getProgramID());
	GLuint LightID = glGetUniformLocation(manager.getProgramID(), "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime(), lastTime2 = glfwGetTime();
	int nbFrames = 0;
	int simplify = 1;
	double lastTime3 = glfwGetTime();
	

	// Get a handle for our "LightPosition" uniform
	glUseProgram(manager.getProgramID());
	LightID = glGetUniformLocation(manager.getProgramID(), "LightPosition_worldspace");
	int continuousMeshSimplification = 0;
	MeshSimplification MS;
	
	printInstructions();
	 
	//TwAddButton(g_pToolBar, "Transform model 0", transformModel(NULL, manager, 0) , NULL, "label='Tranform 0' ");
	//TwAddButton(g_pToolBar, "Transform model 0", manager.setModelTransformation(1), NULL, "label='Tranform 0' ");
	//TwAddButton(g_pToolBar, "Transform model 0", manager.setModelTransformation(2), NULL, "label='Tranform 0' ");


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
	
		//Trabalho 1 - Mesh Simplification
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
			MS.reduce(*(*(*manager.getModels())[0].getMesh()).getIndexedVertices(), *(*(*manager.getModels())[0].getMesh()).getIndices(), *(*(*manager.getModels())[0].getMesh()).getIndexedNormals(), continuousMeshSimplification);

			//Bind
			(*(*manager.getModels())[0].getMesh()).rebind();

			// Get a handle for our "LightPosition" uniform
			glUseProgram(manager.getProgramID());
			LightID = glGetUniformLocation(manager.getProgramID(), "LightPosition_worldspace");

			//End Bind
		
		}
		else if (((currentTime2 >= lastTime2 + 0.5) && (glfwGetKey(g_pWindow, GLFW_KEY_B) == GLFW_PRESS) && continuousMeshSimplification == 0) || continuousMeshSimplification == -1 ) {
			lastTime2 = glfwGetTime();
			MS.reconstruct(*(*(*manager.getModels())[0].getMesh()).getIndexedVertices(), *(*(*manager.getModels())[0].getMesh()).getIndices(), continuousMeshSimplification);

			//Bind
			(*(*manager.getModels())[0].getMesh()).rebind();

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
		double currentTime = glfwGetTime();

		/* --- Trabalho 2 --- */
		//Modelo atual
		if (glfwGetKey(g_pWindow, GLFW_KEY_0) == GLFW_PRESS)
			currentModelID = 0;
		else if (glfwGetKey(g_pWindow, GLFW_KEY_1) == GLFW_PRESS)
			currentModelID = 1;
		else if (glfwGetKey(g_pWindow, GLFW_KEY_2) == GLFW_PRESS)
			currentModelID = 2;
		else if (glfwGetKey(g_pWindow, GLFW_KEY_3) == GLFW_PRESS)
			currentModelID = 3;


		//Translação ao pressionar T
		if (glfwGetKey(g_pWindow, GLFW_KEY_T) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID != -1) {	//Translation
			lastTime3 = glfwGetTime();
			(*manager.getModels())[currentModelID].addTransformation(translationVector, translationTime, 'T', 0);
			std::cout << "Queue size:" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (glfwGetKey(g_pWindow, GLFW_KEY_S) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID != -1) {	//Scale
			lastTime3 = glfwGetTime();
			(*manager.getModels())[currentModelID].addTransformation(glm::vec3(scaleVector, scaleVector, scaleVector), scaleTime, 'S', 0);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (glfwGetKey(g_pWindow, GLFW_KEY_R) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID != -1) {	//Rotation
			lastTime3 = glfwGetTime();
			(*manager.getModels())[currentModelID].addTransformation(rotationDirection, rotationTime, 'R', rotationDegrees);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (glfwGetKey(g_pWindow, GLFW_KEY_P) == GLFW_PRESS && (currentTime > lastTime3 + 0.1) && currentModelID != -1) {	//Rotation Around point (not working)
			lastTime3 = glfwGetTime();
			//(*manager.getModels())[currentModelID].addCompTransformation(rotationDirection, rotationTime, 'R', rotationDegrees, translationVector, translationTime, 'T', 0);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (glfwGetKey(g_pWindow, GLFW_KEY_I) == GLFW_PRESS && (currentTime > lastTime3 + 0.1) && currentModelID != -1) {	//Rotação + Translação + Escala
			lastTime3 = glfwGetTime();
			(*manager.getModels())[currentModelID].addCompTransformation(glm::vec3(scaleVector, scaleVector, scaleVector), compositionTime, 'S', 0, translationVector, compositionTime, 'T', 0, rotationDirection, compositionTime, 'R', rotationDegrees);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (glfwGetKey(g_pWindow, GLFW_KEY_H) == GLFW_PRESS && (currentTime > lastTime3 + 0.3) && currentModelID != -1) {	//Shear ('H')
			lastTime3 = glfwGetTime();
			(*manager.getModels())[currentModelID].addTransformation(shearVector, shearTime, 'H', 0);
			std::cout << "Queue size of model" << currentModelID << ":" << (*(*manager.getModels())[currentModelID].getTransformationQueue()).size() << std::endl;
		}
		else if (glfwGetKey(g_pWindow, GLFW_KEY_DELETE) == GLFW_PRESS){	//Resets all  input
			translationVector = glm::vec3(0, 0, 0);
			translationTime = 0;
			scaleVector = 0;
			scaleTime = 0;
			rotationDirection = glm::vec3(0, 0, 0);
			rotationDegrees = 0;
			rotationTime = 0;
			shearVector = glm::vec3(0, 0, 0);
			shearTime = 0;
		}
		else if (glfwGetKey(g_pWindow, GLFW_KEY_U) == GLFW_PRESS && (currentTime > lastTime3 + 0.1) && currentModelID != -1) {	//Apply all
			lastTime3 = glfwGetTime();
			manager.setModelTransformation(currentModelID);
		}


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

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(nUseMouse, g_nWidth, g_nHeight);
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		//::mat4 ModelMatrix      = model.getModelMatrix();
		//glm::mat4 MVP = ProjectionMatrix * ViewMatrix * (*manager.getModels())[0].getModelMatrix();

		manager.drawModels(ViewMatrixID, ViewMatrix, ProjectionMatrix, g_pWindow);

		/**
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(manager.getProgramID());

		

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(model.getModelMatrixID(), 1, GL_FALSE, &model.getModelMatrix()[0][0]);
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
		*/

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

