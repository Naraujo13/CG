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

class Projectile : Model {
private:
	long double lastTimeMoved;
	long double speed;
public:
	//Construtor
	Projectile::Projectile(const char *textPath, const char *textSample, GLuint programID, std::vector<Mesh> meshes, glm::vec3 position, std::string type);
	void moveProjectile();
};

#endif