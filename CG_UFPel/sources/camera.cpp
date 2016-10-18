#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#include <glm/gtx/transform2.hpp>

#include <camera.hpp>
#include <modelManager.hpp>


#define STEPS 40 //number of steps of an animations
#define LOD 100//Level of detail of the curve

//Construtor
Camera::Camera(GLuint programID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix)
{
	ViewMatrixID = glGetUniformLocation(programID, "V");
	Camera::ViewMatrix = ViewMatrix;
	Camera::ProjectionMatrix = ProjectionMatrix;
	Camera::timeBtwn = 0;
	Camera::lastTransformed = 0;
	Camera::state = 0;
}

//Getters
glm::mat4 Camera::getProjectionMatrix() {
	return ProjectionMatrix;
}

glm::mat4 Camera::getViewMatrix() {
	return ViewMatrix;
}

GLuint Camera::getViewMatrixID() {
	return ViewMatrixID;
}

std::vector<Transformation> * Camera::getTransformationQueue() {
	return &transformationQueue;
}
int Camera::getState() {
	return state;
}
long double Camera::getLastTransformed() {
	return lastTransformed;
}

long double Camera::getTimeBtwn() {
	return timeBtwn;
}

//Setters
void Camera::setViewMatrix(glm::mat4 ViewMatrix) {
	Camera::ViewMatrix = ViewMatrix;
}

void Camera::setProjectionMatrix(glm::mat4 ProjectionMatrix) {
	Camera::ProjectionMatrix = ProjectionMatrix;
}
void Camera::setState(int newState) {
	Camera::state = newState;
}

//Transformations

void Camera::addCompTransformation(struct translation *t, struct rotation *r, struct cameraLookAt *lk){

	//Split and push all the transformations to queue with the time between them	
	int firstFlag = 1;
	glm::mat4 stepTransformation(1.0);

	std::cout << "Before type test!" << std::endl;

	//Translation
	if (t != NULL) {
		//Animation time control
		int steps = ceil(t->time * STEPS);
		if (steps <= 0)
			steps = 1;
		long double stepTime = (long double)t->time / (long double)steps - 0.005;

		stepTransformation = glm::translate(stepTransformation, t->translationVec / glm::vec3(steps, steps, steps) * glm::vec3(-1));
		// Push all transformations
		for (int i = 0; i < steps; i++) {
			if (firstFlag != 0) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else {
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
			}
		}
	}
	//Rotation
	else if (r != NULL) {
		//Animation time control
		int steps = ceil(r->time * STEPS);
		if (steps <= 0)
			steps = 1;
		long double stepTime = (long double)r->time / (long double)steps - 0.005;

		stepTransformation = glm::rotate(stepTransformation, (float) r->rotationDegrees / steps, r->rotationVec);
		// Push all transformations
		for (int i = 0; i < steps; i++) {
			if (firstFlag != 0) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else 
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
		}
		
	}
	//LookAt
	if (t== NULL && r == NULL && lk != NULL) {
		//Animation time control
		int steps = ceil(lk->time * STEPS);
		if (steps <= 0)
			steps = 1;
		long double stepTime = (long double)lk->time / (long double)steps - 0.005;

		std::cout << "Before animating!" << std::endl;
		stepTransformation = glm::lookAt(lk->eye, lk->center, lk->up);
		std::cout << "After animating!" << std::endl;
		// Push all transformations
		for (int i = 0; i < steps; i++) {
			if (firstFlag != 0) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else {
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
			}
		}
	}
}

void Camera::applyTransformation() {
	if (transformationQueue.empty()) {
		Camera::state = 0;
		std::cout << "Camera transformation queue empty." << std::endl;
		return;
	}
	else if (glfwGetTime() > lastTransformed + timeBtwn) {
		
			//Aplica operação
			setViewMatrix(transformationQueue.front().getTransformation() * ViewMatrix);

			//Apaga operação e atualiza controle
			transformationQueue.erase(transformationQueue.begin());
			lastTransformed = glfwGetTime();

		if (!transformationQueue.empty())
			timeBtwn = transformationQueue.front().getTimeBtwn();

		else
			timeBtwn = 0;
	}
}
