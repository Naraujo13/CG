#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#define STEPS 20 //number of steps of an animations
//Constructor
Model::Model(const char *textPath, const char *textSampler, GLuint programID, Mesh &modelMesh)
{
	texture = loadDDS(textPath);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	mesh = &modelMesh;
	modelMatrixID = glGetUniformLocation(programID, "M");
	modelMatrix = glm::mat4(1);
	state = 0;
	timeBtwn = 0;
	lastTransformed = glfwGetTime();
}

//Getters
GLuint Model::getModelMatrixID() {
	return modelMatrixID;
}
GLuint Model::getTextureID() {
	return textureID;
}
GLuint* Model::getTexture() {
	return &texture;
}
glm::mat4 Model::getModelMatrix() {
	return modelMatrix;
}
Mesh* Model::getMesh() {
	return mesh;
}
std::vector<Transformation> Model::getTransformationQueue() {
	return transformationQueue;
}
int Model::getState() {
	return state;
}

//Setter
void Model::setModelMatrix(glm::mat4 matrix) {
	Model::modelMatrix = matrix;
}
void Model::setState(int newState) {
	Model::state = newState;
}

//Others
void Model::addTransformation(glm::vec3 transformation, double time) {
	//Split and push all the transformations to queue with the time between them
	double stepTime = time / STEPS;
	glm::vec3 stepTransformation = transformation / (glm::vec3 (STEPS, STEPS, STEPS));
	int firstFlag = 1;

	for (int i = 0; i < STEPS; i++) {
		if (firstFlag == 1) {
			transformationQueue.push_back(Transformation(stepTransformation, 0));
			firstFlag = 0;
		}
		else
			transformationQueue.push_back(Transformation(stepTransformation, stepTime));
	}
	//transformationQueue.push_back(Transformation (transformation, time));
}

void Model::applyTranslation() {
	if (transformationQueue.empty())
		return;
	setModelMatrix(glm::mat4(glm::translate(modelMatrix, transformationQueue.front().getTransformation())));
	transformationQueue.erase(transformationQueue.begin());
	lastTransformed = glfwGetTime();
}



//Translations
/*
void Model::translateModel(glm::mat4 translationMatrix) {
	std::cout << "Matriz de translacao:" << std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << translationMatrix[i][j] << "|";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << "Matriz do objeto antes:" << std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << modelMatrix[i][j] << "|";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl;


	glm::vec3 translate = glm::vec3(50.0f,1.0f,1.0f);
	modelMatrix = glm::translate(modelMatrix,  translate);
	/*
	if (modelMatrix == modelMatrix)
		std::cout << "Nao deu certo. Matriz continua igual." << std::endl;
	else
		std::cout << "Deu certo. Matriz diferente." << std::endl;
	*/
/*	std::cout << "Matriz do objeto depois:" << std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << modelMatrix[i][j] << "|";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl;
	
	}
*/