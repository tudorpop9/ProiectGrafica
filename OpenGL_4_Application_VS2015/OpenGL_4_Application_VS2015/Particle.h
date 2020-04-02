#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"

#include "Shader.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

struct Particle {
	glm::vec2 Position;
	glm::vec2 Velocity;
	glm::vec4 Color;
	GLfloat lifeTime;

	Particle() : Position(0.0f), Velocity(0.0f), Color(0.0f), lifeTime(0.0f) {}
};

struct ParticleGenerator
{
	std::vector<Particle> particles;
	glm::vec2 initPos;
	glm::vec2 initVel;
	GLuint amount;
	// Render state
	gps::Shader shader;
	GLuint VAO;
};

void Update(GLfloat dt, GLuint newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
void init(ParticleGenerator *pg);
void Draw(ParticleGenerator *pg);
GLuint firstUnusedParticle(ParticleGenerator *pg);
void respawnParticle(Particle &particle, glm::vec2 initPos, glm::vec2 initVel, glm::vec2 offset = glm::vec2(0.0f, 0.0f));


void Update(ParticleGenerator *pg, float dmg, GLuint newParticles, glm::vec2 offset) {

	for (GLuint i = 0; i < newParticles; i++) {
		int unusedParticle = firstUnusedParticle(pg);
		respawnParticle(pg->particles[unusedParticle], pg->initPos, pg->initVel, offset);
	}

	for (GLuint i = 0; i < pg->amount; i++) {
		Particle &p = pg->particles[i];
		p.lifeTime -= dmg;
		if (p.lifeTime > 0.0f) {
			p.Position -= p.Velocity*dmg;
			p.Color -= dmg * 2.5;
		}
	}
}

int offsetLoc;
int colorLoc;
void Draw(ParticleGenerator *pg, glm::vec2 offset) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	pg->shader.useShaderProgram();
	for (Particle particle : pg->particles) {
		if (particle.lifeTime > 0.0f) {
			offsetLoc = glGetUniformLocation(pg->shader.shaderProgram, "offset");
			colorLoc = glGetUniformLocation(pg->shader.shaderProgram, "color");
			glUniform4fv(colorLoc, 1, particle.Color);
			glUniform2fv(colorLoc, 1,  offset);

			glBindVertexArray(pg->VAO);	
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void init(ParticleGenerator *pg) {
	// Set up mesh and attribute properties
	GLuint VBO;
	GLfloat particle_quad[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};
	glGenVertexArrays(1, &pg->VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(pg->VAO);
	// Fill mesh buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

	// Set mesh attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	for (GLuint i = 0; i < pg->amount; i++) {
		pg->particles.push_back(Particle());
	}
}

// Stores the index of the last particle used (for quick access to next dead particle)
GLuint lastUsedParticle = 0;
GLuint firstUnusedParticle(ParticleGenerator* pg)
{
	// First search from last used particle, this will usually return almost instantly
	for (GLuint i = lastUsedParticle; i < pg->amount; ++i) {
		if (pg->particles[i].lifeTime <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// Otherwise, do a linear search
	for (GLuint i = 0; i < lastUsedParticle; ++i) {
		if (pg->particles[i].lifeTime <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
	lastUsedParticle = 0;
	return 0;
}

void respawnParticle(Particle &particle, glm::vec2 initPos, glm::vec2 initVel, glm::vec2 offset)
{
	GLfloat random = (GLfloat)((rand() % 100) - 50) / 10.0f;
	GLfloat rColor = (GLfloat)0.5 + ((rand() % 100) / 100.0f);
	particle.Position = initPos + random + offset;
	particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
	particle.lifeTime = 1.0f;
	particle.Velocity = initVel * 0.1f;
}

/*class ParticleGenerator
{
public:
	// Constructor
	ParticleGenerator(gps::Shader shader, glm::vec2 initPos, glm::vec2 initVel,GLuint amount);
	// Update all particles
	void Update(GLfloat dt, GLuint newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
	// Render all particles
	void Draw();
private:
	// State
	std::vector<Particle> particles;
	glm::vec2 initPos;
	glm::vec2 initVel;
	GLuint amount;
	// Render state
	gps::Shader shader;
	//glm::Texture2D texture;
	GLuint VAO;
	// Initializes buffer and vertex attributes
	void init();
	// Returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
	GLuint firstUnusedParticle();
	// Respawns particle
	void respawnParticle(Particle &particle, glm::vec2 initPos, glm::vec2 initVel, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};*/