#include "ParticleSystem.h"
#include "Particle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assert.h>

ParticleSystem::ParticleSystem()
	: mInitialized(false)
	, mCurrentReadBuffer(0)
	, mTransformFeedbackBuffer(0)
	, mQuery(0)
	, mTexture(0)
{
	mVbos[0] = 0;
	mVaos[0] = 0;
}

ParticleSystem::~ParticleSystem()
{
	if (mTransformFeedbackBuffer != 0)
	{
		glDeleteTransformFeedbacks(1, &mTransformFeedbackBuffer);
	}
	if (mQuery != 0)
	{
		glDeleteQueries(1, &mQuery);
	}
	if (mVbos[0] != 0)
	{
		glDeleteBuffers(sBufferSize, mVbos);
	}
	if (mVaos[0] != 0)
	{
		glDeleteVertexArrays(sBufferSize, mVaos);
	}
	//delete &mTexture;
}

bool ParticleSystem::InitalizeParticleSystem()
{
	assert(!mInitalized && "Particle system already initalized!");

	const char* sVaryings[] = {
		"vPositionOut",
		"vVelocityOut",
		"vColorOut",
		"vLifeTimeOut",
		"vSizeOut",
		"vTypeOut"
	};
	unsigned int varyingSize = sizeof(sVaryings) / sizeof(sVaryings[0]);
	mParticleUpdateShader = new Shader("C:/Users/lukas/Documents/Studium_Technikum/SPG/SPG_Project/SPG_Projekt/SPG_Projekt/shader/ParticleUpdate.vs", nullptr, "C:/Users/lukas/Documents/Studium_Technikum/SPG/SPG_Project/SPG_Projekt/SPG_Projekt/shader/ParticleUpdate.gs");
	mParticleRenderShader = new Shader("C:/Users/lukas/Documents/Studium_Technikum/SPG/SPG_Project/SPG_Projekt/SPG_Projekt/shader/ParticleRender.vs", "C:/Users/lukas/Documents/Studium_Technikum/SPG/SPG_Project/SPG_Projekt/SPG_Projekt/shader/ParticleRender.frag", "C:/Users/lukas/Documents/Studium_Technikum/SPG/SPG_Project/SPG_Projekt/SPG_Projekt/shader/ParticleRender.gs");

	for (int i = 0; i < varyingSize; i++)
	{
		glTransformFeedbackVaryings(mParticleUpdateShader->ID, varyingSize, sVaryings, GL_INTERLEAVED_ATTRIBS);
	}
	mParticleUpdateShader->link();

	glGenTransformFeedbacks(1, &mTransformFeedbackBuffer);
	glGenQueries(1, &mQuery);

	glGenBuffers(sBufferSize, mVbos);
	glGenVertexArrays(sBufferSize, mVaos);

	Particle initParticle;
	initParticle.Type = ParticleType::GENERATOR;

	for (int i = 0; i < sBufferSize; i++)
	{
		glBindVertexArray(mVaos[i]);
		glBindBuffer(GL_ARRAY_BUFFER, mVbos[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * sMaxParticles, nullptr, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), &initParticle);

		for (unsigned int j = 0; j < varyingSize; j++)
		{
			glEnableVertexAttribArray(j);
		}

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)12);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)24);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)36);
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)40);
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)44);
	}
	mCurrentReadBuffer = 0;
	mNumParticles = 1;

	mInitialized = true;
	return true;
}

float grandf(float fMin, float fAdd)
{
	float fRandom = float(rand() % (RAND_MAX + 1)) / float(RAND_MAX);
	return fMin + fAdd * fRandom;
}

void ParticleSystem::UpdateParticles(float timeStep)
{
	CheckInit();
	mParticleUpdateShader->use();

	mParticleUpdateShader->setVec3("uGenPosition", mGenPosition);
	mParticleUpdateShader->setVec3("uGenVelocityMin", mGenVelocityMin);
	mParticleUpdateShader->setVec3("uGenVelocityRange", mGenVelocityRange);
	mParticleUpdateShader->setVec3("uGenColor", mGenColor);
	mParticleUpdateShader->setVec3("uGenGravityVector", mGenGravityVector);

	mParticleUpdateShader->setFloat("uTimeStep", timeStep);
	mParticleUpdateShader->setFloat("uGenLifeTimeMin", mGenLifeMin);
	mParticleUpdateShader->setFloat("uGenLifeTimeRange", mGenLifeRange);
	mParticleUpdateShader->setFloat("uGenSize", mGenSize);

	mElapsedTime += timeStep;

	if (mElapsedTime > mNextGenerationTime)
	{
		mParticleUpdateShader->setInt("uNumToGenerate", mNumToGenerate);
		mElapsedTime -= mNextGenerationTime;

		glm::vec3 randomSeed = glm::vec3(grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f));
		mParticleUpdateShader->setVec3("uRandomSeed", randomSeed);
	}
	else
	{
		mParticleUpdateShader->setInt("uNumToGenerate", 0);
	}
	glEnable(GL_RASTERIZER_DISCARD);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, mTransformFeedbackBuffer);

	glBindVertexArray(mVaos[mCurrentReadBuffer]);
	glEnableVertexAttribArray(1);

	// 1 -  use other buffer
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVbos[1 - mCurrentReadBuffer]);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, mQuery);
	glBeginTransformFeedback(GL_POINTS);

	glDrawArrays(GL_POINTS, 0, mNumParticles);

	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glGetQueryObjectiv(mQuery, GL_QUERY_RESULT, &mNumParticles);

	// toggle buffer
	mCurrentReadBuffer = 1 - mCurrentReadBuffer;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glDisable(GL_RASTERIZER_DISCARD);
}

void ParticleSystem::RenderParticles()
{
	CheckInit();

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	mParticleRenderShader->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	mParticleRenderShader->setMat4("uProjection", mProjection);
	mParticleRenderShader->setMat4("uView", mView);
	mParticleRenderShader->setVec3("uQuad1", mQuad1);
	mParticleRenderShader->setVec3("uQuad2", mQuad2);

	glBindVertexArray(mVaos[mCurrentReadBuffer]);
	glDisableVertexAttribArray(1);

	glDrawArrays(GL_POINTS, 0, mNumParticles);

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void ParticleSystem::SetGeneratorProperties(const glm::vec3& position, const glm::vec3& velocityMin, const glm::vec3& velocityMax, const glm::vec3& gravity, const glm::vec3 color, float minLifeTime, float maxLifeTime, float size, float spawnTime, int numToGenerate)
{
	mGenPosition = position;
	mGenVelocityMin = velocityMin;
	mGenVelocityRange = velocityMax - velocityMin;
	mGenGravityVector = gravity;
	mGenColor = color;

	mGenSize = size;
	mGenLifeMin = minLifeTime;
	mGenLifeRange = maxLifeTime - minLifeTime;

	mNextGenerationTime = spawnTime;
	mElapsedTime = 0.0f;

	mNumToGenerate = numToGenerate;
}

void ParticleSystem::SetGeneratorPosition(const glm::vec3& position)
{
	mGenPosition = position;
}

int ParticleSystem::GetNumParticles()
{
	return mNumParticles;
}

void ParticleSystem::SetMatrices(const glm::mat4& projection, const glm::mat4& viewMat, const glm::vec3& view, const glm::vec3& upVector)
{
	mProjection = projection;
	mView = viewMat;
	mQuad1 = glm::cross(view, upVector);
	mQuad1 = glm::normalize(mQuad1);
	mQuad2 = glm::cross(view, mQuad1);
	mQuad2 = glm::normalize(mQuad2);
}

void ParticleSystem::CheckInit() const
{
	assert(mInitalized && "Particle system not initalized before use");
}