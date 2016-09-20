#pragma once
#ifndef MODEL_HPP
#define MODEL_HPP

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





//Define a transformation
class Transformation{
private:
	glm::mat4 transformation;
	double timeBtwn;
public:
//Constructor
	Transformation::Transformation(glm::mat4 transformation, long double time) {
		Transformation::transformation = transformation;
		Transformation::timeBtwn = time;
	}
//Getters
	glm::mat4 getTransformation() {
		return transformation;
	}
	double getTimeBtwn() {
		return timeBtwn;
	}
};





//Define a custom data type to represent a model
class Model {

private:
	//Model matrix with the current applied matrix to the mesh
	GLuint modelMatrixID;
	glm::mat4 modelMatrix;

	//Queue to store the transformations to be applied to this model
	std::vector<Transformation> transformationQueue;
	int state;	//0 = still, 1 = transforming;
	long double lastTransformed;
	long double timeBtwn;

	//Texture info
	GLuint texture;
	GLuint textureID;

	//Mesh
	Mesh* mesh;

public:
	//Constructor
	Model::Model(const char *textPath, const char *textSample, GLuint programID, Mesh &mesh, glm::vec3 position);
	//Getters
	GLuint getModelMatrixID();
	GLuint getTextureID();
	GLuint *getTexture();
	glm::mat4 getModelMatrix();
	Mesh* getMesh();
	std::vector<Transformation> * getTransformationQueue();
	int getState();
	long double getLastTransformed();
	long double getTimeBtwn();

	//Set
	 void setModelMatrix(glm::mat4 transformation);
	 void setState(int newState);

	//Translations
	void Model::addTransformation(glm::vec3 transformation, double time, char type, float rotationDegrees);
	//Transla��o, rota��o, escala
	void Model::addCompTransformation(struct translation *t, struct rotation *r, struct scale *s, struct shear *h, long double time);
 	void Model::applyTransformation();

	void bezierCurve(struct bezier b);
	void BSplineCurve(struct bspline l);
	void Model::BSplineTest(struct bspline l);

};

#endif