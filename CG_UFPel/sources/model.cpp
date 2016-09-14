#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#define STEPS 1000 //number of steps of an animations
//Constructor
Model::Model(const char *textPath, const char *textSampler, GLuint programID, Mesh &modelMesh, glm::vec3 pos)
{
	texture = loadDDS(textPath);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	mesh = &modelMesh;
	modelMatrixID = glGetUniformLocation(programID, "M");
	modelMatrix = glm::mat4(glm::translate(glm::mat4(1), pos));
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
std::vector<Transformation> * Model::getTransformationQueue() {
	return &transformationQueue;
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
void Model::addTransformation(glm::vec3 transformation, double time, char type) {
	//Split and push all the transformations to queue with the time between them
	double stepTime = time / STEPS;
	
	if (type == 'T') {			//Translação
		glm::vec3 stepTransformation = transformation / (glm::vec3(STEPS, STEPS, STEPS));
		int firstFlag = 1;

		for (int i = 0; i < STEPS; i++) {
			if (firstFlag == 1) {
				transformationQueue.push_back(Transformation(stepTransformation, 0, type));
				firstFlag = 0;
			}
			else
				transformationQueue.push_back(Transformation(stepTransformation, stepTime, type));
		}
	}
	else if (type == 'S') {
		glm::vec3 stepTransformation;
		double raiz = (1.0/(double)STEPS);
		
		stepTransformation.x = pow(transformation.x, raiz) / 2 + 0.5;
		//std::cout << "pow(" << transformation.x << ", " << raiz << std::endl;
		stepTransformation.y = pow(transformation.y, raiz) / 2 + 0.5;
		//std::cout << "pow(" << transformation.y << ", " << raiz << std::endl;
		stepTransformation.z = pow(transformation.z, raiz) / 2 + 0.5;
		//std::cout << "pow(" << transformation.z << ", " << raiz << std::endl;
		//std::cout << pow(transformation.x, raiz) / 2 << "," << pow(transformation.y, raiz) / 2 << "," << pow(transformation.z, raiz) / 2 << std::endl;
		for (int i = 0; i < STEPS; i++) {
			transformationQueue.push_back(Transformation(stepTransformation, stepTime, 'S'));
		}
	}
	else if (type == 'R') {
		transformationQueue.push_back(Transformation(transformation, 1, 'R'));
	}
}

void Model::applyTransformation() {
	if (transformationQueue.empty()){
		Model::state = 0;
		std::cout << "empty" << std::endl;
		return;
	}
	else if (glfwGetTime() > lastTransformed + timeBtwn) {
		//Aplica operação
		if(transformationQueue.front().getType() == 'T')	//Se for uma translação
			setModelMatrix((glm::translate(modelMatrix, transformationQueue.front().getTransformation())));
		else if (transformationQueue.front().getType() == 'S') {	//Se for uma escala
			//std::cout << "("<< transformationQueue.front().getTransformation().x << "," << transformationQueue.front().getTransformation().y << "," << transformationQueue.front().getTransformation().z << ")" << std::endl;
			setModelMatrix(glm::scale(modelMatrix, transformationQueue.front().getTransformation()));
		}

		//Apaga operação e atualiza controle
		transformationQueue.erase(transformationQueue.begin());
		lastTransformed = glfwGetTime();
		if (!transformationQueue.empty())
			timeBtwn = transformationQueue.front().getTimeBtwn();
		else
			timeBtwn = 0;
	}
}
