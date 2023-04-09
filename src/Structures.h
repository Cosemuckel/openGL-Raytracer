#pragma once

#include <glm/glm.hpp>

struct Material {
	glm::vec3 color;
	float smoothness;

	glm::vec4 emission;
};

struct Sphere {
	glm::vec3 center;
	float radius;

	Material material;
};

struct Triangle {
	glm::vec3 v0;
	float pad0;

	glm::vec3 v1;
	float pad1;

	glm::vec3 v2;
	float pad2;

	glm::vec3 edge1;
	float pad3;

	glm::vec3 edge2;
	float pad4;

	glm::vec3 normal;
	float pad5;

	Material material;
};

struct Mesh {
	int firstTriangle;
	int lastTriangle;
	bool wasLoaded;
};

struct Camera {
	glm::vec3 position;
	float pad0;

	glm::vec3 direction;
	float pad1;

	glm::vec3 up;
	float pad2;

	glm::vec3 right;
	float pad3;
};

//The object buffer can be passed as a uniform buffer to the shader
template <int MAX_SPHERES, int MAX_TRIANGLES>
struct tObjectBuffer {

	glm::vec2 resolution; // 1, 2
	int numSpheres; // 3
	int numTriangles; // 4

	int maxBounces;
	int numSamples;
	float jitterStrenght;
	float pad1;

	Camera camera;

	Sphere spheres[MAX_SPHERES];
	Triangle triangles[MAX_TRIANGLES];
};

typedef tObjectBuffer<MAX_SPHERES, MAX_TRIANGLES> ObjectBuffer;