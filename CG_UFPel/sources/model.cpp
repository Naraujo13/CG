#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#define STEPS 100 //number of steps of an animations
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
	int steps = (int)time * STEPS;
	double stepTime = time / steps;
	
	if (type == 'T') {	//Translation
		glm::vec3 stepTransformationVec = transformation / (glm::vec3(steps, steps, steps));
		glm::mat4 stepTransformation(1.0);
		stepTransformation[3][0] = stepTransformationVec.x;
		stepTransformation[3][1] = stepTransformationVec.y;
		stepTransformation[3][2] = stepTransformationVec.z;

		int firstFlag = 1;

		for (int i = 0; i < steps; i++) {
			if (firstFlag == 1) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
		}
	}
	else if (type == 'S') {	//Scale
		//glm::vec3 stepTransformationVec;
		glm::mat4 stepTransformation(1.0);
		double raiz = (1.0/(double)steps);
		

		//stepTransformationVec.x = pow(transformation.x, raiz) / 2 + 0.5;
		//stepTransformationVec.y = pow(transformation.y, raiz) / 2 + 0.5;
		//stepTransformationVec.z = pow(transformation.z, raiz) / 2 + 0.5;

		stepTransformation[0][0] = pow(transformation.x, raiz) / 2 + 0.5;
		stepTransformation[1][1] = pow(transformation.y, raiz) / 2 + 0.5;
		stepTransformation[2][2] = pow(transformation.z, raiz) / 2 + 0.5;

		for (int i = 0; i < steps; i++) {
			transformationQueue.push_back(Transformation(stepTransformation, stepTime));
		}
	}
	else if (type == 'R') {	//Rotation
		int firstFlag = 1;
		//double rotationDegreesStep = rotationDegrees / steps;
		
		glm::mat4 stepTransformation(1.0);

		stepTransformation = glm::rotate(stepTransformation, rotationDegrees/steps, transformation);
		for (int i = 0; i < steps; i++) {
			if (firstFlag == 1) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
		}
	}
	else if (type == 'H') {	//Shear

		int firstFlag = 1;
		glm::vec3 stepTransformation = transformation / (glm::vec3(steps, steps, steps));

		glm::mat4 shearMatrix(1.0);
		shearMatrix[0][1] = stepTransformation.x;
		shearMatrix[0][2] = stepTransformation.y;
		shearMatrix[0][3] = stepTransformation.z;


		//transformationQueue.push_back(Transformation(transformation, 0, type, 0));
		for (int i = 0; i < steps; i++) {
			if (firstFlag == 1) {
				transformationQueue.push_back(Transformation(shearMatrix, 0));
				firstFlag = 0;
			}
			else
				transformationQueue.push_back(Transformation(shearMatrix, stepTime));
		}
	}
	else if (type = 'B') {	//Bezier curve
		int firstFlag = 1;
	}
}


void Model::addCompTransformation(glm::vec3 transformation, double time, char type, float rotationDegrees, glm::vec3 transformation2, double time2, char type2, float rotationDegrees2, glm::vec3 transformation3, double time3, char type3, float rotationDegrees3) {
	//Split and push all the transformations to queue with the time between them
	int steps = (int)time * STEPS;
	double stepTime = time / steps;
	//glm::vec3 stepTransformation;
	//double rotationDegreesStep = 0;
	//double stepTime2 = time2 / steps;
	//glm::vec3 stepTransformation2;
	//double rotationDegreesStep2 = 0;
	//double stepTime3 = time3 / steps;
	//glm::vec3 stepTransformation3;
	//double rotationDegreesStep3 = 0;

	glm::mat4 stepTransformation(1.0);

	//Transformation 1
	if (type == 'T') {			//Translação
		stepTransformation = glm::translate(stepTransformation, transformation / glm::vec3(steps, steps, steps));
	}
	else if (type == 'S') {
		double raiz = (1.0 / (double)steps);
		stepTransformation = glm::scale(stepTransformation, glm::vec3(pow(transformation.x, raiz) / 2 + 0.5, pow(transformation.y, raiz) / 2 + 0.5, pow(transformation.z, raiz) / 2 + 0.5));
	
	}
	else if (type == 'R') {
		stepTransformation = glm::rotate(stepTransformation, rotationDegrees / steps, transformation);
	}
	
	//Transformation 2
	if (type2 == 'T') {			//Translação
		stepTransformation = glm::translate(stepTransformation, transformation2 / glm::vec3(steps, steps, steps));
	}
	else if (type2 == 'S') {
		double raiz = (1.0 / (double)steps);
		stepTransformation = glm::scale(stepTransformation, glm::vec3(pow(transformation2.x, raiz) / 2 + 0.5, pow(transformation2.y, raiz) / 2 + 0.5, pow(transformation2.z, raiz) / 2 + 0.5));
	}
	else if (type2 == 'R') {
		stepTransformation = glm::rotate(stepTransformation, rotationDegrees2 / steps, transformation2);
	}

	//Transformation 3
	if (type3 == 'T') {			//Translação
		stepTransformation = glm::translate(stepTransformation, transformation3 / glm::vec3(steps, steps, steps));
	}
	else if (type3 == 'S') {
		double raiz = (1.0 / (double)steps);
		stepTransformation = glm::scale(stepTransformation, glm::vec3(pow(transformation3.x, raiz) / 2 + 0.5, pow(transformation3.y, raiz) / 2 + 0.5, pow(transformation3.z, raiz) / 2 + 0.5));
	}
	else if (type3 == 'R') {
		stepTransformation = glm::rotate(stepTransformation, rotationDegrees3 / steps, transformation3);
	}



	//Push all transformations
	int firstFlag = 1;
	for (int i = 0; i < steps; i++) {
		if (firstFlag != 0) {
			transformationQueue.push_back(Transformation(stepTransformation, 0));
			firstFlag=0;
		}
		else {
			transformationQueue.push_back(Transformation(stepTransformation, stepTime/3));
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
		setModelMatrix(modelMatrix * transformationQueue.front().getTransformation());

		//Apaga operação e atualiza controle
		transformationQueue.erase(transformationQueue.begin());
		lastTransformed = glfwGetTime();
		if (!transformationQueue.empty())
			timeBtwn = transformationQueue.front().getTimeBtwn();
		else
			timeBtwn = 0;
	}
}
