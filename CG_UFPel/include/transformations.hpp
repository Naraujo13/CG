#pragma once
#ifndef TRANSFORMATIONS_HPP
#define TRANSFORMATIONS_HPP

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

//Transformation auxiliares
struct translation {
	glm::vec3 translationVec;
	long double time;
};
struct scale {
	glm::vec3 scaleVec;
	long double time;
};
struct rotation {
	glm::vec3 rotationVec;
	float rotationDegrees;
	long double time;
};
struct rotationAP {
	glm::vec3 point;
	float rotationAngle;
	glm::vec3 rotationAxis;
	long double time;
};
struct shear {
	glm::vec3 shearVec;
	long double time;
};
struct bezier {
	glm::vec3 controlPoints[3];
	long double time;
};
struct bspline {
	glm::vec3 controlPoints[4];
	long double time;
};
struct extraProjection3D {
	glm::vec3 projVector;
	long double time;
};
struct cameraLookAt {
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	long double time;
};



//Define a transformation
class Transformation{
private:
	glm::mat4 transformation;
	double timeBtwn;
	bool lookAtFlag;

public:
//Constructor
	Transformation::Transformation(glm::mat4 transformation, long double time) {
		Transformation::transformation = transformation;
		Transformation::timeBtwn = time;
		lookAtFlag = false;
	}
//Getters
	glm::mat4 getTransformation() {
		return transformation;
	}
	double getTimeBtwn() {
		return timeBtwn;
	}
	bool getLookAtFlag() {
		return lookAtFlag;
	}
//Setters
	void setLookAtFlag(bool b) {
		lookAtFlag = b;
	}
};


#endif