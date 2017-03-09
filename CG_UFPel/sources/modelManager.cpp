#include "modelManager.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#include <shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "controls.hpp"
#include "objloader.hpp"


//Constructor
ModelManager::ModelManager()
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	currentCamera = 0;
}

//Getters
std::vector<Shader> * ModelManager::getShaders() {
	return &shaders;
}
std::vector<Camera> * ModelManager::getCameras() {
	return &cameras;
}
std::vector<Model> * ModelManager::getModels() {
	return &models;
}
std::vector<Mesh> * ModelManager::getMeshes() {
	return &meshes;
}
GLuint ModelManager::getProgramID() {
	return currentShaderProgramID;
}
GLuint ModelManager::getVertexArrayID() {
	return VertexArrayID;
}
GLuint ModelManager::getMatrixID() {
	return MatrixID;
}
GLuint ModelManager::getLightID() {
	return LightID;
}


//creates a new shader (without geometry) and adds to the vector
void ModelManager::createShader(const GLchar* vertex_file_path, const GLchar* fragment_file_path) {
	shaders.push_back(Shader(vertex_file_path, fragment_file_path));
}
//creates a new shader (with geometry) and adds to the vector
void ModelManager::createShader(const GLchar* vertex_file_path, const GLchar* fragment_file_path, const GLchar* geometry_file_path) {
	shaders.push_back(Shader(vertex_file_path, fragment_file_path, geometry_file_path));
}
//creates a new model and adds to the vector
void ModelManager::createModel(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position) {
	models.push_back(Model(textPath, textSampler, currentShaderProgramID, meshes, position));
}

void ModelManager::loadMeshes(std::string path) {
	std::vector <Mesh> meshes;
	meshes = loadAssImp(path);
	printf("Returned from assimp loader: %d meshes\n", meshes.size());
	for (auto it = meshes.begin(); it != meshes.end(); ++it) {
		this->meshes.push_back(*it);
	}
	printf("Number of meshes at manager: %d meshes\n", this->meshes.size());

	//Check?
}

//creates a new camera and adds to the vector
void ModelManager::createCamera(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix) {
	cameras.push_back(Camera(currentShaderProgramID, ViewMatrix, ProjectionMatrix));
}
void ModelManager::createCamera(float fieldOfView, float aspectRatio, float near, float far, glm::vec3 cameraPosition, glm::vec3 upVector, glm::vec3 sightDirection)
{
	cameras.push_back(Camera( fieldOfView, aspectRatio, near, far, cameraPosition, upVector, sightDirection, currentShaderProgramID));
}

//Shader functions
bool ModelManager::useShader(int index) {
	if (index >= 0 && index < shaders.size()) {
		currentShaderProgramID = shaders[index].programID;
		shaders[index].use();
		LightID = glGetUniformLocation(currentShaderProgramID, "LightPosition_worldspace");
		MatrixID = glGetUniformLocation(currentShaderProgramID, "MVP");
		return true;
	}
	else {
		std::cout << "ERROR::INVALID SHADER INDEX::MAX_INDEX==" << shaders.size() << "::DESIRED_INDEX==" << index << "::ERROR" << std::endl;
		return false;
	}
}

//Camera functions
void ModelManager::changeCurrentCamera(int newCamera){
	if (newCamera < cameras.size())
		currentCamera = newCamera;
}


void ModelManager::cleanup() {

	for (auto it = meshes.begin(); it != meshes.end(); ++it) {
		(*it).cleanup();
	
	}
	for (auto it = models.begin(); it != models.end(); ++it) {
		//Delete Texture
		glDeleteTextures(1, (*it).getTexture());
	}
	glDeleteProgram(currentShaderProgramID);
	glDeleteVertexArrays(1, &VertexArrayID);

}

void ModelManager::clearScreen() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ModelManager::swapBuffers(GLFWwindow* g_pWindow) {
	// Get a handle for our "LightPosition" uniform
	glUseProgram(currentShaderProgramID);
	LightID = glGetUniformLocation(currentShaderProgramID, "LightPosition_worldspace");
	// Swap buffers			
	glfwSwapBuffers(g_pWindow);
	glfwPollEvents();
}

void ModelManager::drawModels(GLuint ViewMatrixID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, GLFWwindow* g_pWindow) {
	
	// Use our shader
	glUseProgram(currentShaderProgramID);

	//For each model in the manager 
	for (auto it = models.begin(); it != models.end(); ++it) {
		//Calculate MVP matrix
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * (*it).getModelMatrix();

		//Geometry Shader Data
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "projection"), 1, GL_FALSE, glm::value_ptr(cameras[currentCamera].getProjectionMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(cameras[currentCamera].getViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(it->getModelMatrix()));
		if (it->getGeometry()) {			
			long double time = (it->getLastUsedGeometry() + (glfwGetTime() - it->getGeometryStart()));
			glUniform1f(glGetUniformLocation(currentShaderProgramID, "time"), time);
			std::cout << "DEBUG::SHADER:: | Time " << time << " | ::SHADER::DEBUG" << std::endl;
		}
		else {
			glUniform1f(glGetUniformLocation(currentShaderProgramID, "time"), it->getLastUsedGeometry());
		}

		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv((*it).getModelMatrixID(), 1, GL_FALSE, &(*it).getModelMatrix()[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, *(*it).getTexture());
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i((*it).getTextureID(), 0);

		//Load/Draw meshes
		for (auto it2 = (*it->getMeshes()).begin(); it2 < (*it->getMeshes()).end(); ++it2) {

			//Load mesh
			it2->loadMesh();

			// Draw the triangles !
			glDrawElements(
				GL_TRIANGLES,        // mode
				(*it2->getIndices()).size(),      // count
				GL_UNSIGNED_SHORT,   // type
				(void*)0             // element array buffer offset
			);

		}		

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Draw tweak bars
		TwDraw();
	}
}

void ModelManager::transformModels() {
	int i = 0;
	for (auto it = ModelManager::models.begin(); it != ModelManager::models.end(); ++it, i++) {
		if (it->getState()) {
			it->applyTransformation();
		}
	}
}

void ModelManager::transformCameras() {
	int i = 0;
	for (auto it = ModelManager::cameras.begin(); it != ModelManager::cameras.end(); ++it, i++) {
		if (it->getState()) {
			it->applyTransformation();
		}
	}
}

/**/
GLboolean ModelManager::checkCollision(Model A, Model B)
{
	bool collisionX = false;
	bool collisionY = false;

	if ((A.getPosition().x + (*A.getMeshes())[0].getSize().x / 2) >= B.getPosition().x && ((B.getPosition().x + (*B.getMeshes())[0].getSize().x / 2) >= A.getPosition().x))
	{
		collisionX = true;
	}
	if ((A.getPosition().y + (*A.getMeshes())[0].getSize().y / 2) >= B.getPosition().y && ((B.getPosition().y + (*B.getMeshes())[0].getSize().y / 2) >= A.getPosition().y))
	{
		collisionY = true;
	}
	return collisionX && collisionY;
}

void ModelManager::cameraNoise() {
	noiseSteps = 40;
	noiseCount = 0;
	glm::vec3 noise;
	glm::mat4 noiseMatrix;
	//Camera Shake
	//Noise
	if (glfwGetTime() > lastNoise + noiseInterval && (cameras[currentCamera].getState())) {
		if (noiseSteps == noiseCount) {
			//Se já fez 10 passos deste noise, recalcula novo noise
			//Gera Pontos Aleatórios
			noise.x = 1 * sin((float)(rand() % 360)) / 15;
			noise.y = 1 * sin((float)(rand() % 360)) / 15;
			noise.z = 1 * sin((float)(rand() % 360)) / 15;
			//noise.z = 0;

			noiseMatrix = glm::translate(glm::mat4(1.0f), noise / glm::vec3(noiseSteps));

			noiseInterval = 1 / noiseSteps;
			noiseCount = 0;
		}

		//Translada fazendo noise
		cameras[currentCamera].setViewMatrix(cameras[currentCamera].getViewMatrix()*noiseMatrix);

		//Atualiza controle
		noiseCount++;
		lastNoise = glfwGetTime();
	}
}

void ModelManager::setModelTransformation(int modelID) {
	if (models.size() >= modelID) {
		models.at(modelID).setState(1);
		std::cout << "State of model " << modelID << " is now "<< models.at(modelID).getState() << ". Model is tranforming." << std::endl;
	}
}

void ModelManager::setCamerasTransformation(int cameraID) {
	if (cameras.size() >= cameraID) {
		cameras.at(cameraID).setState(1);
		std::cout << "State of model " << cameraID << " is now " << cameras.at(cameraID).getState() << ". Model is tranforming." << std::endl;
	}
}

