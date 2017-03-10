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
std::vector<Model> * ModelManager::getEnemies() {
	return &enemies;
}
std::vector<Model> * ModelManager::getPlayers() {
	return &players;
}
std::vector<Model> * ModelManager::getProjectiles() {
	return &projectiles;
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
void ModelManager::createModel(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position, std::string type) {
	Model model(textPath, textSampler, currentShaderProgramID, meshes, position, type);
	models.push_back(model);
	if (type == "Enemy")
		enemies.push_back(model);
	else if (type == "Projectile")
		projectiles.push_back(model);
	else if (type == "Player")
		players.push_back(model);
	else if (type == "Scenerie")
		sceneries.push_back(model);
}

void ModelManager::loadMeshes(std::string path) {
	std::vector <Mesh> meshes;
	meshes = loadAssImp(path);
	printf("Returned from assimp loader: %d meshes\n", meshes.size());
	for (auto it = meshes.begin(); it != meshes.end(); ++it) {
		this->meshes.push_back(*it);
	}
	printf("Number of meshes at manager: %d meshes\n", this->meshes.size());
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

	//Draw players
	for (auto it = players.begin(); it != players.end(); ++it) {
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

	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
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

	for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
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

void  ModelManager::setTransformPlayers(bool newState) {
	transformPlayers = newState;
}
void  ModelManager::setTransformEnemies(bool newState) {
	transformEnemies = newState;
}
void  ModelManager::setTransformProjectiles(bool newState) {
	transformProjectiles = newState;
}
void  ModelManager::setTransformScenerie(bool newState) {
	transformSceneries = newState;
}

void ModelManager::transformModels() {
	int i = 0;
	if (transformPlayers) {
		for (auto it = ModelManager::players.begin(); it != ModelManager::players.end(); ++it, i++) {
			if (it->getState()) {
				it->applyTransformation();
			}
		}
	}
	if (transformEnemies) {
		for (auto it = ModelManager::enemies.begin(); it != ModelManager::enemies.end(); ++it, i++) {
			if (it->getState()) {
				it->applyTransformation();
			}
		}
	}
	if (transformProjectiles) {
		for (auto it = ModelManager::projectiles.begin(); it != ModelManager::projectiles.end(); ++it, i++) {
			if (it->getState()) {
				it->applyTransformation();
			}
		}
	}
	if (transformSceneries) {
		for (auto it = ModelManager::sceneries.begin(); it != ModelManager::sceneries.end(); ++it, i++) {
			if (it->getState()) {
				it->applyTransformation();
			}
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


GLboolean ModelManager::checkCollision(glm::vec3 positionA, glm::vec3 sizeA, glm::vec3 positionB, glm::vec3 sizeB)
{
	
	bool collisionX = false;
	bool collisionY = false;

	//glm::vec3 positionA = A.getPosition();
	//glm::vec3 positionB = B.getPosition();

	if (positionA.x + (sizeA.x / 2) >= positionB.x && ((positionB.x + sizeB.x / 2) >= positionA.x))
	{
		collisionX = true;
	}
	if (positionA.y + (sizeA.y / 2) >= positionB.y && ((positionB.y + (sizeB.y / 2) >= positionA.y)))
	{
		collisionY = true;
	}

	

	std::cout <<std::endl<< "Model A position: (" << positionA.x << ", " << positionA.y << ", " << positionA.z << ")";
	std::cout << " | Model B posiition (" << positionB.x << ", " << positionB.y << ", " << positionB.z << ")" << std::endl;

	std::cout << "Collision X = " << collisionX << " | Collision Y = " << collisionY << std::endl;
	return collisionX && collisionY;
}


/* -- Colisões -- */
void ModelManager::checkAllModelsCollision() {

	std::cout << "Starting to check collisions..." << std::endl;

	//Enemy-Player Collision
	std::cout << "\tChecking Enemy-Player collisions... ";
	if (!enemies.empty()) {
		for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy) {
			if (enemy->isAlive()){
				for (auto player = players.begin(); player != players.end(); ++player) {
					if (player->isAlive()) {
						bool playerCollision = checkCollision(player->getPosition(),  (*player->getMeshes())[0].getSize(), enemy->getPosition(), (*enemy->getMeshes())[0].getSize());
						std::cout << playerCollision << std::endl;
						if (playerCollision) {
							std::cout << "----Detected Enemy-Player collision.----" << std::endl;
							enemy->setAlive(!playerCollision);
							player->setAlive(!playerCollision);
							player->setGeometry(true);
							enemy->setGeometry(true);
							//END GAME HERE
						}
					}
				}
			}
		}
	}


	if (!projectiles.empty()) {
		for (auto projectile = projectiles.begin(); projectile != models.end(); ++projectile) {
			if (projectile->isAlive()) {

				std::cout << "\tChecking Projectile-Player collisions..." << std::endl;
				//Player-Projectile Collision
				for (auto player = players.begin(); player != players.end(); ++player) {
					if (player->isAlive()) {
						bool collision = checkCollision(player->getPosition(), (*player->getMeshes())[0].getSize(), projectile->getPosition(), (*projectile->getMeshes())[0].getSize());
						if (collision) {
							std::cout << "----Detected Projectile-Player collision.----" << std::endl;
							getchar();
							player->setAlive(!collision);
							projectile->setAlive(!collision);
							//END GAME HERE
						}
					}
				}

				std::cout << "\tChecking Enemy-Projectile collisions..." << std::endl;
				//Enemy-Projectile Collision
				for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy) {
					if (enemy->isAlive()) {
						bool collision = checkCollision(enemy->getPosition(), (*enemy->getMeshes())[0].getSize(), projectile->getPosition(), (*projectile->getMeshes())[0].getSize());
						if (collision) {
							std::cout << "----Detected Enemy-Player collision.----" << std::endl;
							getchar();
							enemy->setAlive(!collision);
							projectile->setAlive(!collision);
						}
					}
				}

			}
		}
	}

	std::cout << "Finished checking collisions." << std::endl;
}

void ModelManager::printCollisions() {
	int i = 0;
	std::cout << "----- Colisões: -----" << std::endl;
	for (auto it = models.begin(); it != models.end(); ++it) {
		std::cout << "Modelo " << i << " está ";
		if (it->isAlive())
			std::cout << " vivo." << std::endl;
		else
			std::cout << " morto." << std::endl;
		i++;
	}
	std::cout << std::endl;
}

/* -- Transformation -- */

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
		//models.at(modelID).setState(1);
		if (models.at(modelID).getType() == "Enemy")
			enemies.at(0).setState(1);
		else if (models.at(modelID).getType() == "Player")
			players.at(0).setState(1);
		else if (models.at(modelID).getType() == "Projectile")
			projectiles.at(0).setState(1);
		//std::cout << "State of model " << modelID << " is now "<< models.at(modelID).getState() << ". Model is tranforming." << std::endl;
	}
}

void ModelManager::setCamerasTransformation(int cameraID) {
	if (cameras.size() >= cameraID) {
		cameras.at(cameraID).setState(1);
		std::cout << "State of model " << cameraID << " is now " << cameras.at(cameraID).getState() << ". Model is tranforming." << std::endl;
	}
}

