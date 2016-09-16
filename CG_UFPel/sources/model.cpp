#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#define STEPS 500 //number of steps of an animations
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
void Model::addTransformation(glm::vec3 transformation, double time, char type, float rotationDegrees) {
	//Split and push all the transformations to queue with the time between them
	double stepTime = time / STEPS;
	
	if (type == 'T') {			//Translação
		glm::vec3 stepTransformation = transformation / (glm::vec3(STEPS, STEPS, STEPS));
		int firstFlag = 1;

		for (int i = 0; i < STEPS; i++) {
			if (firstFlag == 1) {
				transformationQueue.push_back(Transformation(stepTransformation, 0, type, 0));
				firstFlag = 0;
			}
			else
				transformationQueue.push_back(Transformation(stepTransformation, stepTime, type, 0));
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
			transformationQueue.push_back(Transformation(stepTransformation, stepTime, 'S', 0));
		}
	}
	else if (type == 'R') {
		int firstFlag = 1;
		double rotationDegreesStep = rotationDegrees / STEPS;
		for (int i = 0; i < STEPS; i++) {
			if (firstFlag == 1) {
				transformationQueue.push_back(Transformation(transformation, 0, type, rotationDegreesStep));
				firstFlag = 0;
			}
			else
				transformationQueue.push_back(Transformation(transformation, stepTime, type, rotationDegreesStep));
		}
	}
}

void Model::addCompTransformation(glm::vec3 transformation, double time, char type, float rotationDegrees, glm::vec3 transformation2, double time2, char type2, float rotationDegrees2) {
	//Split and push all the transformations to queue with the time between them
	double stepTime = time / STEPS;
	glm::vec3 stepTransformation;
	double rotationDegreesStep = 0;
	double stepTime2 = time2 / STEPS;
	glm::vec3 stepTransformation2;
	double rotationDegreesStep2 = 0;


	//Transformation 1
	if (type == 'T') {			//Translação
		stepTransformation = transformation / (glm::vec3(STEPS, STEPS, STEPS));
	}
	else if (type == 'S') {
		double raiz = (1.0 / (double)STEPS);
		stepTransformation.x = pow(transformation.x, raiz) / 2 + 0.5;

		stepTransformation.y = pow(transformation.y, raiz) / 2 + 0.5;

		stepTransformation.z = pow(transformation.z, raiz) / 2 + 0.5;
	
	}
	else if (type == 'R') {
		rotationDegreesStep = rotationDegrees / STEPS;
	}

	//Transformation 2
	if (type2 == 'T') {			//Translação
		stepTransformation2 = transformation2 / (glm::vec3(STEPS, STEPS, STEPS));
	}
	else if (type2 == 'S') {
		double raiz = (1.0 / (double)STEPS);
		stepTransformation2.x = pow(transformation2.x, raiz) / 2 + 0.5;

		stepTransformation2.y = pow(transformation2.y, raiz) / 2 + 0.5;

		stepTransformation2.z = pow(transformation2.z, raiz) / 2 + 0.5;

	}
	else if (type2 == 'R') {
		stepTransformation2 = transformation2;
		rotationDegreesStep2 = rotationDegrees2 / STEPS;
	}


	//Do both transformations
	int firstFlag = 1;
	for (int i = 0; i < STEPS; i++) {
		if (firstFlag > 0) {
			transformationQueue.push_back(Transformation(stepTransformation, 0, type, rotationDegreesStep));
			transformationQueue.push_back(Transformation(stepTransformation2, 0, type2, rotationDegreesStep2));
			firstFlag=0;
		}
		else {
			transformationQueue.push_back(Transformation(stepTransformation, stepTime, type, rotationDegreesStep));
			transformationQueue.push_back(Transformation(stepTransformation2, 0, type2, rotationDegreesStep2));
		}
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
		else if (transformationQueue.front().getType() == 'R') {
			setModelMatrix(glm::rotate(modelMatrix, transformationQueue.front().getRotationDegrees(),transformationQueue.front().getTransformation()));
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
