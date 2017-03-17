#pragma once
#ifndef MODELMANAGER_HPP
#define MODELMANAGER_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <AntTweakBar.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <glerror.hpp>
#include "model.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "projectile.hpp"
#include "enemy.hpp"

//Define a custom data type to represent a model
class ModelManager {

private:
	//Vector to store shaders
	std::vector<Shader> shaders;
	//Vector to store the meshes
	std::vector<Mesh> meshes;
	//Vector to store the cameras
	std::vector<Camera> cameras;
	//Current Camera
	int currentCamera;

	//Noise control
	long double lastNoise = 0;
	long double noiseInterval = 0.25;
	int noiseSteps = 40;
	int noiseCount = 0;

	GLuint VertexArrayID;
	GLuint currentShaderProgramID;
	GLuint MatrixID;
	GLuint LightID;

	//Game Related
	double difficulty;
	//Enemies
	bool transformEnemies;
	std::string currentEnemyPattern;
	long double downPatternStart = 0.0f;
	long double downPatternDuration = 0.1f;
	std::vector<Enemy> enemies;
	//Projectiles
	bool transformProjectiles;
	std::vector<Projectile> projectiles;
	//Sceneries
	bool transformSceneries;
	std::vector<Model> sceneries;
	//Players
	bool transformPlayers;
	std::vector<Model> players;

public:
	//Constructor
	ModelManager::ModelManager();
	//Getters
	std::vector<Shader> * getShaders();
	std::vector<Camera> * getCameras();
	std::vector<Enemy> * ModelManager::getEnemies();
	std::vector<Model> * ModelManager::getPlayers();
	std::vector<Projectile> * ModelManager::getProjectiles();
	std::vector<Mesh> * getMeshes();
	GLuint getVertexArrayID();
	GLuint getProgramID();
	GLuint getMatrixID();
	GLuint getLightID();
	double ModelManager::getDifficulty();
	
	//Shaders
	void createShader(const GLchar* vertex_file_path, const GLchar* fragment_file_path);
	void createShader(const GLchar* vertex_file_path, const GLchar* fragment_file_path, const GLchar* geometry_file_path);
	bool useShader(int index);

	//Camera
	void createCamera(glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix);
	void createCamera(float fieldOfView, float aspectRatio, float near, float far, glm::vec3 cameraPosition, glm::vec3 upVector, glm::vec3 sightDirection);
	void changeCurrentCamera(int newCamera);
	void transformCameras();
	void cameraNoise();
	void setCamerasTransformation(int cameraID);
	
	//mesh
	void loadMeshes(std::string path);

	//models
	void createModel(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position, std::string type);
	void ModelManager::addModel(Model model);
	void ModelManager::addModel(Enemy model);
	void ModelManager::addModel(Projectile model);
	void cleanup();
	void clearScreen();
	void swapBuffers(GLFWwindow* g_pWindow);
	void drawModels(GLuint ViewMatrixID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, GLFWwindow* g_pWindow);
	void transformModels();
	void setTransformPlayers(bool newState);
	void setTransformEnemies(bool newState);
	void setTransformProjectiles(bool newState);
	void setTransformScenerie(bool newState);
	void setModelTransformation(int modelID, std::string type);

	//Collision
	GLboolean ModelManager::checkCollision(glm::vec3 positionA, glm::vec3 sizeA, glm::vec3 positionB, glm::vec3 sizeB, std::string type);
	void printCollisions();
	void checkAllModelsCollision();
	void deadModelsCollector();

	//Enemies
	void ModelManager::createEnemy(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position, long double speedPerSecond, int health);
	void ModelManager::updateEnemyMovementPattern();
	void ModelManager::movesEnemyInPattern();

	//Projectiles
	void ModelManager::createProjectile(char *textPath, char *textSampler, std::vector<Mesh> meshes, glm::vec3 position, long double speedPerSecond);
	void ModelManager::projectilesMovementPattern();
};

#endif