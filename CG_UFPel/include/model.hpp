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
#include "transformations.hpp"

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

	//Geometry
	bool geometry;
	long double geometryStart;
	long double lastUsedGeometry;

	//Texture info
	GLuint texture;
	GLuint textureID;

	//Mesh
	std::vector<Mesh> meshes;

	//Collision
	bool alive;
	long double timeOfDeath = 0;
	glm::vec3 position;
	glm::vec3 collisionBox;
	glm::vec3 size;
	

	//Game Related info
	std::string type;	//'Player', 'Enemy', 'Projectile', 'Scenery'

public:
	//Constructor
	Model::Model(const char *textPath, const char *textSample, GLuint programID, std::vector<Mesh> meshes, glm::vec3 position, std::string type);
	//Getters
	GLuint getModelMatrixID();
	GLuint getTextureID();
	GLuint *getTexture();
	glm::mat4 getModelMatrix();
	std::vector<Mesh> * getMeshes();
	std::vector<Transformation> * getTransformationQueue();
	int getState();
	long double getLastTransformed();
	long double getTimeBtwn();
	bool getGeometry();
	long double getGeometryStart();
	long double getLastUsedGeometry();
	glm::vec3 getPosition();
	bool isAlive();
	std::string getType();
	long double getTimeOfDeath();
	

	//Set
	 void setModelMatrix(glm::mat4 transformation);
	 void setState(int newState);
	 void setGeometry(bool newState);
	 void setGeometryStart(long double time);
	 void setLastUsedGeometry(long double time);
	 void setAlive(bool newState);


	//Transformations
	void Model::addCompTransformation(struct translation *t, struct rotation *r, struct scale *s, struct shear *h, struct extraProjection3D *p3D, long double time);
 	void Model::applyTransformation();
	void Model::rotationAroundPoint(struct rotationAP *p);
	void bezierCurve(struct bezier b);
	void Model::BSplineTest(struct bspline l);

	double getScalingFactor();
	glm::vec3 Model::getSize();
	void calculateSize();

};

#endif