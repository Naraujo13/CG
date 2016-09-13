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

//Define a transformation
class Transformation{
private:
	glm::vec3 transformation;
	double timeBtwn;
public:
//Constructor
	Transformation::Transformation(glm::vec3 transformation, double time) {
		Transformation::transformation = transformation;
		Transformation::timeBtwn = time;
	}
//Getters
	glm::vec3 getTransformation() {
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
	double lastTransformed;
	double timeBtwn;

	//Texture info
	GLuint texture;
	GLuint textureID;

	//Mesh
	Mesh* mesh;

public:
	//Constructor
	Model::Model(const char *textPath, const char *textSample, GLuint programID, Mesh &mesh);
	//Getters
	GLuint getModelMatrixID();
	GLuint getTextureID();
	GLuint *getTexture();
	glm::mat4 getModelMatrix();
	Mesh* getMesh();
	std::vector<Transformation> getTransformationQueue();
	int getState();
	double getLastTransformed();
	double getTimeBtwn();

	//Set
	 void setModelMatrix(glm::mat4 transformation);
	 void setState(int newState);

	//Translations
	void Model::addTransformation(glm::vec3 transformation, double time);
	void Model::applyTranslation();



};

#endif