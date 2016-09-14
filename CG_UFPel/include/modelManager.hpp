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

//Define a custom data type to represent a model
class ModelManager {

private:
	//Vector to store the models
	std::vector<Mesh> meshes;
	//Vector to store the meshes
	std::vector<Model> models;

	GLuint VertexArrayID;
	GLuint programID;
	GLuint MatrixID;
	GLuint LightID;

public:
	//Constructor
	ModelManager::ModelManager(char *shaderPath1, char *shaderPath2);
	//Getters
	std::vector<Model> * getModels();
	std::vector<Mesh> * getMeshes();
	GLuint getVertexArrayID();
	GLuint getProgramID();
	GLuint getMatrixID();
	GLuint getLightID();
	//models
	void createModel(char *textPath, char *textSampler, Mesh &mesh, glm::vec3 position);
	void createMesh(char *path);
	void cleanup();
	void drawModels(GLuint ViewMatrixID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix, GLFWwindow* g_pWindow);
	void transformModels();
	void setModelTransformation(int modelID);
};

#endif