#pragma once
#ifndef PROJECTILE_HPP
#define PROJECTILE_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include "mesh.hpp"
#include "model.hpp"

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <glerror.hpp>

class Projectile : public Model {
private:
	long double lastTimeMoved;
	long double stepTime;
	long double speedPerSecond;
	long double speedPerStep;
public:
	//Construtor
	Projectile::Projectile(const char * textPath, const char * textSample, GLuint programID, std::vector<Mesh> meshes, glm::vec3 position, std::string type, long double speed);
	void moveProjectile();
	void checkBoundaries();
};

#endif