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
	double time;
};
struct scale {
	glm::vec3 scaleVec;
	double time;
};
struct rotation {
	glm::vec3 rotationVec;
	double rotationDegrees;
	double time;
};
struct shear {
	glm::vec3 shearVec;
	double time;
};


//Define a transformation
class Transformation{
private:
	glm::mat4 transformation;
	//float rotationDegrees;
	double timeBtwn;
	//char type;	//usado para controle interno. T - Translação, R - rotação, S - shear, P - rotação ao redor de ponto, E  rotação ao redor de eixo, B - bspine, Z - bezier
public:
//Constructor
	Transformation::Transformation(glm::mat4 transformation, double time) {
		Transformation::transformation = transformation;
		Transformation::timeBtwn = time;
//		Transformation::type = type;
//		Transformation::rotationDegrees = rotationDegrees;
	}
//Getters
	glm::mat4 getTransformation() {
		return transformation;
	}
	double getTimeBtwn() {
		return timeBtwn;
	}
//	char getType() {
//		return type;
//	}
//	float getRotationDegrees() {
//		return rotationDegrees;
//	}
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
	Model::Model(const char *textPath, const char *textSample, GLuint programID, Mesh &mesh, glm::vec3 position);
	//Getters
	GLuint getModelMatrixID();
	GLuint getTextureID();
	GLuint *getTexture();
	glm::mat4 getModelMatrix();
	Mesh* getMesh();
	std::vector<Transformation> * getTransformationQueue();
	int getState();
	double getLastTransformed();
	double getTimeBtwn();

	//Set
	 void setModelMatrix(glm::mat4 transformation);
	 void setState(int newState);

	//Translations
	void Model::addTransformation(glm::vec3 transformation, double time, char type, float rotationDegrees);
	void Model::addCompTransformation(glm::vec3 transformation, double time, char type, float rotationDegrees, glm::vec3 transformation2, double time2, char type2, float rotationDegrees2, glm::vec3 transformation3, double time3, char type3, float rotationDegrees3);
	void Model::applyTransformation();
	


};

#endif