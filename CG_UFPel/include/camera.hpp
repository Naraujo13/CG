#pragma once
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include "mesh.hpp"
#include <AntTweakBar.h>

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <glerror.hpp>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <glerror.hpp>

class Camera {
private:
	//Camera attributes
	GLuint ViewMatrixID;
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;
	//Transformations
	std::vector<Transformation> transformationQueue;

	//Animation Control
	int state;	//0 = still, 1 = transforming;
	long double lastTransformed;
	long double timeBtwn;

public:
	//Construtor
	Camera::Camera(GLuint programID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix);

	//Getters
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	GLuint getViewMatrixID();
	std::vector<Transformation> * getTransformationQueue();
	int getState();
	long double getLastTransformed();
	long double getTimeBtwn();

	//Setters
	void setViewMatrix(glm::mat4);
	void setProjectionMatrix(glm::mat4);
	void setState(int newState);

	//Transformation
	void Camera::addCompTransformation(struct translation *t, struct rotation *r, struct cameraLookAt *l);
	void Camera::applyTransformation();
};

#endif