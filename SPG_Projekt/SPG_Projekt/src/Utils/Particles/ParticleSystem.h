#pragma once

#include <glm/glm.hpp>
#include "../Shader.h"

#include <string>

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	bool InitalizeParticleSystem();

	void RenderParticles();
	void UpdateParticles(float fTimePassed);

	void SetGeneratorProperties(const glm::vec3& position, const glm::vec3& velocityMin, const glm::vec3& velocityMax, const glm::vec3& gravity, const glm::vec3 color, float minLifeTime, float maxLifeTime, float size, float spawnTime, int numToGenerate);
	void SetGeneratorPosition(const glm::vec3& position);


	void SetMatrices(const glm::mat4& projection, const glm::mat4& viewMat, const glm::vec3& view, const glm::vec3& upVector);

	int GetNumParticles();
	unsigned int mTexture;
	float mNextGenerationTime;
	//CParticleSystemTransformFeedback();

private:
	void CheckInit() const;

	static const int sBufferSize = 2;
	static const int sMaxParticles = 100000;

	bool mInitialized;

	unsigned int mTransformFeedbackBuffer;

	//unsigned int mParticleBuffer[sBufferSize];
	//unsigned int mVAO[sBufferSize];
	unsigned int mVbos[sBufferSize];
	unsigned int mVaos[sBufferSize];

	unsigned int mQuery;

	int mCurrentReadBuffer;
	int mNumParticles;

	glm::mat4 mProjection, mView;
	glm::vec3 mQuad1, mQuad2;

	float mElapsedTime;

	glm::vec3 mGenPosition;
	glm::vec3 mGenVelocityMin;
	glm::vec3 mGenVelocityRange;
	glm::vec3 mGenGravityVector;
	glm::vec3 mGenColor;

	float mGenLifeMin;
	float mGenLifeRange;
	float mGenSize;

	int mNumToGenerate;

	Shader* mParticleRenderShader;
	Shader* mParticleUpdateShader;
};