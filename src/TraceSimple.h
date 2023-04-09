#pragma once

#include <glm/glm.hpp>
#include <math.h>

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
};

int raySphere(const Ray& ray, const Sphere& sphere) {
	//Returns the distance to the intersection point
	//Returns -1 if there is no intersection

	glm::vec3 oc = ray.origin - sphere.center;
	float a = glm::dot(ray.direction, ray.direction);
	float b = 2.0f * glm::dot(oc, ray.direction);
	float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;

	float discriminant = b * b - 4 * a * c;
	
	return (discriminant <= 0) ? -1 : (-b - sqrt(discriminant)) / (2.0f * a);	
}

int rayTriangle(const Ray& ray, const Triangle& triangle) {
	//Returns the distance to the intersection point
	//Returns -1 if there is no intersection

	glm::vec3 pvec = glm::cross(ray.direction, triangle.edge2);
	float det = glm::dot(triangle.edge1, pvec);

	if (det < 0.0001f) return -1;

	float invDet = 1.0f / det;

	glm::vec3 tvec = ray.origin - triangle.v0;
	float u = glm::dot(tvec, pvec) * invDet;
	if (u < 0.0f || u > 1.0f) return -1;

	glm::vec3 qvec = glm::cross(tvec, triangle.edge1);
	float v = glm::dot(ray.direction, qvec) * invDet;
	if (v < 0.0f || u + v > 1.0f) return -1;

	return glm::dot(triangle.edge2, qvec) * invDet;
}

int getIndexAt(const glm::vec2 coordinate, const ObjectBuffer& objectBuffer) {
	//Returns the index of the closet clicked object
	int closestObject = -1;
	float closestDistance = 10e10f;
	
	Ray ray;
	ray.origin = objectBuffer.camera.position;
	ray.direction = glm::normalize(objectBuffer.camera.direction + coordinate.x * objectBuffer.camera.right + coordinate.y * objectBuffer.camera.up);

	for (int i = 0; i < objectBuffer.numSpheres; i++) {
		float distance = raySphere(ray, objectBuffer.spheres[i]);
		if (distance > 0 && distance < closestDistance) {
			closestDistance = distance;
			closestObject = i;
		}
	}

	for (int i = 0; i < objectBuffer.numTriangles; i++) {
		float distance = rayTriangle(ray, objectBuffer.triangles[i]);
		if (distance > 0 && distance < closestDistance) {
			closestDistance = distance;
			closestObject = i + objectBuffer.numSpheres;
		}
	}

	return closestObject;
}