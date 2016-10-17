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


//Construtor
Camera::Camera(GLuint programID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix)
{
	ViewMatrixID = glGetUniformLocation(programID, "V");
	Camera::ViewMatrix = ViewMatrix;
	Camera::ProjectionMatrix = ProjectionMatrix;
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



//setters
void Camera::setViewMatrix(glm::mat4 ViewMatrix) {
	Camera::ViewMatrix = ViewMatrix;
}

void Camera::setProjectionMatrix(glm::mat4 ProjectionMatrix) {
	Camera::ProjectionMatrix = ProjectionMatrix;
}

//Transformations

void Camera::addCompTransformation(struct translation *t){
	glm::mat4 transformation = glm::translate(glm::mat4(1.0f), t->translationVec * glm::vec3(-1));
	transformationQueue.push_back(transformation);
}

void Camera::applyTransformation() {
	if (transformationQueue.empty()) {
		return;
	}
	else {
		//Aplica operação
		setViewMatrix(ViewMatrix * transformationQueue.front());

		//Apaga operação e atualiza controle
		transformationQueue.erase(transformationQueue.begin());
	}
}
