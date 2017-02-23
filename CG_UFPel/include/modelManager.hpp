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

//Define a custom data type to represent a model
class ModelManager {

private:
	//Vector to store shaders
	std::vector<Shader> shaders;
	//Vector to store the models
	std::vector<Mesh> meshes;
	//Vector to store the meshes
	std::vector<Model> models;
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

public:
	//Constructor
	ModelManager::ModelManager();
	//Getters
	std::vector<Shader> * getShaders();
	std::vector<Camera> * getCameras();
	std::vector<Model> * getModels();
	std::vector<Mesh> * getMeshes();
	GLuint getVertexArrayID();
	GLuint getProgramID();
	GLuint getMatrixID();
	GLuint getLightID();
	
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
	void createMesh(char *path);

	//models
	void createModel(char *textPath, char *textSampler, Mesh &mesh, glm::vec3 position);
	void cleanup();
	void clearScreen();
	void swapBuffers(GLFWwindow* g_pWindow);
	void drawModels(GLuint ViewMatrixID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, GLFWwindow* g_pWindow);
	void transformModels();
	void setModelTransformation(int modelID);
};

#endif