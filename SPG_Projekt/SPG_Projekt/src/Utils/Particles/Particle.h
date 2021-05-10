#pragma once

#include <glm/glm.hpp>

enum ParticleType
{
	GENERATOR = 0,
	PARTICLE = 1
};

struct Particle
{
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec3 Color;
	float LifeTime;
	float Size;
	float Type;
};