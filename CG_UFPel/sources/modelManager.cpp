#include "modelManager.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <projectile.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#include <shader.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "controls.hpp"
#include "objloader.hpp"

#define GEOMETRYLIMIT 0.0f

//Boundaries
#define LEFTBOUNDARY -16.0f
#define RIGHTBOUNDARY 16.0f
#define TOPBOUNDARY 12.0f
#define BOTTOMBOUNDARY 12.0f

//Dead Models Time Limits
#define CLEANPROJECTILES 2.0f
#define CLEANENEMIES 5.0f


//Constructor
ModelManager::ModelManager()
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	currentCamera = 0;
	difficulty = 1;
	currentEnemyPattern = "R";
}

/* -- Getters -- */
std::vector<Shader> * ModelManager::getShaders() {
	return &shaders;
}
std::vector<Camera> * ModelManager::getCameras() {
	return &cameras;
}
std::vector<Enemy> * ModelManager::getEnemies() {
	return &enemies;
}
std::vector<Model> * ModelManager::getPlayers() {
	return &players;
}
std::vector<Projectile> * ModelManager::getProjectiles() {
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
double ModelManager::getDifficulty() {
	return difficulty;
}
/* ------------- */

/* -- Models -- */
//creates a new model and adds to the vector
void ModelManager::createModel(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position, std::string type) {
	Model model(textPath, textSampler, currentShaderProgramID, meshes, position, type);
	if (type == "Player")
		players.push_back(model);
	else if (type == "Scenerie")
		sceneries.push_back(model);
}

void ModelManager::addModel(Model model) {
	if (model.getType() == "Player")
		players.push_back(model);
	else if (model.getType() == "Scenerie")
		sceneries.push_back(model);
}

void ModelManager::addModel(Projectile model) {
	projectiles.push_back(model);
}
void ModelManager::addModel(Enemy model) {
	enemies.push_back(model);
}

/* ------------ */

/* -- Meshes -- */
void ModelManager::loadMeshes(std::string path) {
	std::vector <Mesh> meshes;
	meshes = loadAssImp(path);
	printf("Returned from assimp loader: %d meshes\n", meshes.size());
	for (auto it = meshes.begin(); it != meshes.end(); ++it) {
		this->meshes.push_back(*it);
	}
	printf("Number of meshes at manager: %d meshes\n", this->meshes.size());
}
/* ------------ */

/* -- Shader -- */
//creates a new shader (without geometry) and adds to the vector
void ModelManager::createShader(const GLchar* vertex_file_path, const GLchar* fragment_file_path) {
	shaders.push_back(Shader(vertex_file_path, fragment_file_path));
}

//creates a new shader (with geometry) and adds to the vector
void ModelManager::createShader(const GLchar* vertex_file_path, const GLchar* fragment_file_path, const GLchar* geometry_file_path) {
	shaders.push_back(Shader(vertex_file_path, fragment_file_path, geometry_file_path));
}

//Troca shader atual
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
/* ----------- */


/* -- Câmeras -- */
//Cria uma nova câmera com 2 parâemtros
void ModelManager::createCamera(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix) {
	cameras.push_back(Camera(currentShaderProgramID, ViewMatrix, ProjectionMatrix));
}

//Cria uma câmera com 6 parâmetros
void ModelManager::createCamera(float fieldOfView, float aspectRatio, float near, float far, glm::vec3 cameraPosition, glm::vec3 upVector, glm::vec3 sightDirection)
{
	cameras.push_back(Camera(fieldOfView, aspectRatio, near, far, cameraPosition, upVector, sightDirection, currentShaderProgramID));
}

//Muda câmera atual
void ModelManager::changeCurrentCamera(int newCamera) {
	if (newCamera < cameras.size())
		currentCamera = newCamera;
}
/* ------------ */


/* -- Draw -- */
//Desenha modelos
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
			if (time < GEOMETRYLIMIT) {
				glUniform1f(glGetUniformLocation(currentShaderProgramID, "time"), time);
			}
			else
				it->setGeometry(false);
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

	//Draw enemies
	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		//Calculate MVP matrix
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * (*it).getModelMatrix();

		//Geometry Shader Data
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "projection"), 1, GL_FALSE, glm::value_ptr(cameras[currentCamera].getProjectionMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(cameras[currentCamera].getViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(it->getModelMatrix()));
		if (it->getGeometry()) {
			long double time = (it->getLastUsedGeometry() + (glfwGetTime() - it->getGeometryStart()));
			if (time < GEOMETRYLIMIT) {
				glUniform1f(glGetUniformLocation(currentShaderProgramID, "time"), time);
			}
			else
				it->setGeometry(false);
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

	//Draw projectiles
	for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
		//Calculate MVP matrix
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * (*it).getModelMatrix();

		//Geometry Shader Data
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "projection"), 1, GL_FALSE, glm::value_ptr(cameras[currentCamera].getProjectionMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "view"), 1, GL_FALSE, glm::value_ptr(cameras[currentCamera].getViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(currentShaderProgramID, "model"), 1, GL_FALSE, glm::value_ptr(it->getModelMatrix()));
		if (it->getGeometry()) {
			long double time = (it->getLastUsedGeometry() + (glfwGetTime() - it->getGeometryStart()));
			if (time < GEOMETRYLIMIT) {
				glUniform1f(glGetUniformLocation(currentShaderProgramID, "time"), time);
			}
			else
				it->setGeometry(false);
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

//Swap buffers
void ModelManager::swapBuffers(GLFWwindow* g_pWindow) {
	// Get a handle for our "LightPosition" uniform
	glUseProgram(currentShaderProgramID);
	LightID = glGetUniformLocation(currentShaderProgramID, "LightPosition_worldspace");
	// Swap buffers			
	glfwSwapBuffers(g_pWindow);
	glfwPollEvents();
}

//Cleans Screen
void ModelManager::clearScreen() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//Final cleanup
void ModelManager::cleanup() {

	//Player
	for (auto player = players.begin(); player != players.end(); ++player) {
		for (auto mesh = (*player->getMeshes()).begin(); mesh != (*player->getMeshes()).begin(); ++mesh)
			(*mesh).cleanup();
		glDeleteTextures(1, (*player).getTexture());
	}

	//Enemies
	for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy) {
		for (auto mesh = (*enemy->getMeshes()).begin(); mesh != (*enemy->getMeshes()).begin(); ++mesh)
			(*mesh).cleanup();
		glDeleteTextures(1, (*enemy).getTexture());
	}

	//Projectiles
	for (auto projectile = projectiles.begin(); projectile != projectiles.end(); ++projectile) {
		for (auto mesh = (*projectile->getMeshes()).begin(); mesh != (*projectile->getMeshes()).begin(); ++mesh)
			(*mesh).cleanup();
		glDeleteTextures(1, (*projectile).getTexture());
	}

	//Scenerie
	for (auto scenery = sceneries.begin(); scenery != sceneries.end(); ++scenery) {
		for (auto mesh = (*scenery->getMeshes()).begin(); mesh != (*scenery->getMeshes()).begin(); ++mesh)
			(*mesh).cleanup();
		glDeleteTextures(1, (*scenery).getTexture());
	}

	glDeleteProgram(currentShaderProgramID);
	glDeleteVertexArrays(1, &VertexArrayID);

}
/* ---------- */


/* -- Colisões -- */
//Verifica colisão entre dois modelos
GLboolean ModelManager::checkCollision(glm::vec3 positionA, glm::vec3 sizeA, glm::vec3 positionB, glm::vec3 sizeB, std::string type)
{

	bool collisionX = false;
	bool collisionY = false;
	bool collisionZ = false;

	if (positionA.x + (sizeA.x / 2) >= positionB.x && ((positionB.x + sizeB.x / 2) >= positionA.x))
	{
		collisionX = true;
	}
	if (positionA.y + (sizeA.y / 2) >= positionB.y && ((positionB.y + (sizeB.y / 2) >= positionA.y)))
	{
		collisionY = true;
	}
	//if (positionA.z + (sizeA.z / 2) >= positionB.z && ((positionB.z + (sizeB.z / 2) >= positionA.z)))
	//{
	//	collisionZ = true;
	//}
	return collisionX && collisionY;
}

//Verifica colisão entre todos os modelos
void ModelManager::checkAllModelsCollision() {

	//Enemy-Player Collision
	if (!enemies.empty()) {
		for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy) {
			if (enemy->isAlive()){
				for (auto player = players.begin(); player != players.end(); ++player) {
					if (player->isAlive()) {
						bool playerCollision = checkCollision(player->getPosition(),  player->getSize(), enemy->getPosition(), enemy->getSize(), "P-E");
						if (playerCollision) {
							player->setAlive(!playerCollision);
							player->setGeometry(true);
							//END GAME HERE
						}
					}
				}
			}
		}
	}


	if (!projectiles.empty()) {
		for (auto projectile = projectiles.begin(); projectile != projectiles.end(); ++projectile) {
			if (projectile->isAlive()) {
				//Enemy-Projectile Collision
				for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy) {
					if (enemy->isAlive()) {
						bool collision = checkCollision(projectile->getPosition(), projectile->getSize(), enemy->getPosition(), enemy->getSize(), "E-P");
						if (collision) {
							enemy->setAlive(!collision);
							projectile->setAlive(!collision);
							enemy->setGeometry(true);
							projectile->setGeometry(true);
						}
					}
				}
			}
		}
	}

}

//Debug para colisoes
void ModelManager::printCollisions() {
	int i = 0;
	std::cout << "----- Colisões: -----" << std::endl;
	for (auto it = players.begin(); it != players.end(); ++it) {
		std::cout << "Player " << i << " está ";
		if (it->isAlive())
			std::cout << " vivo." << std::endl;
		else
			std::cout << " morto." << std::endl;
		i++;
	}
	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		std::cout << "Inimigo " << i << " está ";
		if (it->isAlive())
			std::cout << " vivo." << std::endl;
		else
			std::cout << " morto." << std::endl;
		i++;
	}
	for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
		std::cout << "Projétil " << i << " está ";
		if (it->isAlive())
			std::cout << " vivo." << std::endl;
		else
			std::cout << " morto." << std::endl;
		i++;
	}
	for (auto it = sceneries.begin(); it != sceneries.end(); ++it) {
		std::cout << "Cenário " << i << " está ";
		if (it->isAlive())
			std::cout << " vivo." << std::endl;
		else
			std::cout << " morto." << std::endl;
		i++;
	}
	std::cout << std::endl;
}
/* ------------- */


/* -- Transformation -- */
//Estado de transformação para jogadores
void  ModelManager::setTransformPlayers(bool newState) {
	transformPlayers = newState;
}

//EStado de tarnsformação de inimigos
void  ModelManager::setTransformEnemies(bool newState) {

	transformEnemies = newState;
}

//Estado de transformação para projéteis
void  ModelManager::setTransformProjectiles(bool newState) {
	transformProjectiles = newState;
}

//Estado de transformação para cenário
void  ModelManager::setTransformScenerie(bool newState) {
	transformSceneries = newState;
}

//Muda estado de transformação para modelo
void ModelManager::setModelTransformation(int modelID, std::string type) {
	if ((type != "Enemy" && type != "Player" && type != "Projectile" && "Type" != "Scenerie") ||
		(type == "Enemy" && enemies.size() <= modelID) || 
		(type == "Player" && players.size() <= modelID) ||
		(type == "Projectile" && projectiles.size() <= modelID) ||
		(type == "Scenerie" && sceneries.size() <= modelID))
		return;

	if (type == "Enemy")
		enemies.at(modelID).setState(1);
	else if (type == "Player")
		players.at(modelID).setState(1);
	else if (type == "Projectile")
		projectiles.at(modelID).setState(1);
}

//Muda estado de transformação para camera
void ModelManager::setCamerasTransformation(int cameraID) {
	if (cameras.size() >= cameraID) {
		cameras.at(cameraID).setState(1);
		std::cout << "State of model " << cameraID << " is now " << cameras.at(cameraID).getState() << ". Model is tranforming." << std::endl;
	}
}

//Transforma todos os modelos
void ModelManager::transformModels() {
	int i = 0;
	if (transformPlayers) {
		for (auto it = ModelManager::players.begin(); it != ModelManager::players.end(); ++it, i++) {
			it->applyTransformation();
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

//Transforma todas as cameras
void ModelManager::transformCameras() {
	int i = 0;
	for (auto it = ModelManager::cameras.begin(); it != ModelManager::cameras.end(); ++it, i++) {
		if (it->getState()) {
			it->applyTransformation();
		}
	}
}

//Simula noise para cameras
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
/* -------------------- */

/* -- Enemies Control -- */
//Creates Enemy
void ModelManager::createEnemy(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position, long double speedPerSecond, int health) {
	this->enemies.push_back(Enemy(textPath, textSampler, currentShaderProgramID, meshes, position, "Enemy", speedPerSecond, health));
}

void ModelManager::updateEnemyMovementPattern() {
	std::string old = currentEnemyPattern;
	std::string nova;
	long double currentTime;
	//std::cout << "Current Pattern: " << currentEnemyPattern << "\tUpdating pattern.." << std::endl;
	//Movendo-se para esquerda ou direita
	if (currentEnemyPattern == "R" || currentEnemyPattern == "L") {
		bool right = false, left = false;
		glm::vec3 pos, size;
		for (auto it = enemies.begin(); it != enemies.end(); ++it) {
			pos = it->getPosition();
			size = it->getSize();
			if (pos.x + size.x / 2 >= RIGHTBOUNDARY) {
				right = true;
				break;
			}
			else if (pos.x - size.x / 2 <= LEFTBOUNDARY) {
				left = true;
				break;
			}
		}
		if (right) {
			currentEnemyPattern = "DownL";
			downPatternStart = glfwGetTime();
			//Go Down, Then Left
		}
		else if (left) {
			currentEnemyPattern = "DownR";
			downPatternStart = glfwGetTime();
			//Go Down Then Right
		}
	}
	//Movendo-se para baixo
	else if (currentEnemyPattern == "DownR" || currentEnemyPattern == "DownL") {
		//Se já finalizou pattern para baixo, muda para direita ou esquerda
		currentTime = glfwGetTime();
		if (currentTime > downPatternStart + downPatternDuration) {
			if (currentEnemyPattern == "DownR")
				currentEnemyPattern = "R";
			else if (currentEnemyPattern == "DownL")
				currentEnemyPattern = "L";
		}
	}
	//std::cout << "New Pattern: " << currentEnemyPattern << std::endl;
	nova = currentEnemyPattern;
}

//Put new sequence of movements to any enemies alive
void ModelManager::movesEnemyInPattern() {
	std::string direction;

	//Updates pattern
	updateEnemyMovementPattern();

	//Gets current pattern
	if (currentEnemyPattern == "L")
		direction = "Left";
	else if (currentEnemyPattern == "R")
		direction = "Right";
	else if (currentEnemyPattern == "DownR" || currentEnemyPattern == "DownL")
		direction = "Down";

	//Moves
	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		if (it->isAlive())
			it->moveEnemy(direction);
	}
	
}
/* --------------------- */

/* -- Projectiles Control -- */

void ModelManager::createProjectile(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position, long double speedPerSecond) {
	Projectile projectile(textPath, textSampler, currentShaderProgramID, meshes, position, "Projectile", speedPerSecond);
	this->projectiles.push_back(projectile);
}

void ModelManager::projectilesMovementPattern() {
	for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
		if (it->isAlive())
			it->moveProjectile();
	}
}


/* -- General -- */
void ModelManager::deadModelsCollector() {
	//Inimigos
	for (auto it = projectiles.begin(); it != projectiles.end(); ++it) {
		if (!it->isAlive()) {
			if (glfwGetTime() > it->getTimeOfDeath() + CLEANPROJECTILES) {
				projectiles.erase(it);
				std::cout << "Apagou projétil." << std::endl;
			}
		}
	}
	//Projeteis
	for (auto it = enemies.begin(); it != enemies.end(); ++it) {
		if (!it->isAlive()) {
			if (glfwGetTime() > it->getTimeOfDeath() + CLEANENEMIES) {
				enemies.erase(it);
				std::cout << "Apagou inimigo." << std::endl;			
			}
		}
	}
}

/* --------------------- */