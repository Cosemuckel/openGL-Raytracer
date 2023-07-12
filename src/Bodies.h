#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

bool createSphere(ObjectBuffer& objectBuffer, const glm::vec3& center, const float radius, const Material& material) {

	if (objectBuffer.numSpheres >= MAX_SPHERES) {
		std::cerr << "Error: Too many spheres!\n";
		std::cerr << "Maximum is " << MAX_SPHERES << "\n";
		return false;
	}

	Sphere sphere;
	sphere.center = center;
	sphere.radius = radius;
	sphere.material = material;
	objectBuffer.spheres[objectBuffer.numSpheres++] = sphere;
	return true;
}

bool createTriangle(ObjectBuffer& objectBuffer, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const Material& material) {

	if (objectBuffer.numTriangles >= MAX_TRIANGLES) {
		std::cerr << "Error: Too many triangles!\n";
		std::cerr << "Maximum is " << MAX_TRIANGLES << "\n";
		return false;
	}

	Triangle triangle;
	triangle.v0 = v0;
	triangle.v1 = v1;
	triangle.v2 = v2;
	triangle.material = material;
	objectBuffer.triangles[objectBuffer.numTriangles++] = triangle;
	return true;
}

void computeTriangles(ObjectBuffer& objectBuffer) {

#pragma omp parallel for
	for (int i = 0; i < objectBuffer.numTriangles; ++i) {
		Triangle& triangle = objectBuffer.triangles[i];
		triangle.edge1 = triangle.v1 - triangle.v0;
		triangle.edge2 = triangle.v2 - triangle.v0;
		triangle.normal = glm::normalize(glm::cross(triangle.edge1, triangle.edge2));
	}
}

bool loadMesh(ObjectBuffer& objectBuffer, const char* path, Mesh& mesh) {
	
	mesh.wasLoaded = false;
	std::ifstream meshFile(path);
	if (!meshFile.is_open()) {
		std::cout << "Error: Could not open mesh file\n";
		return false;
	}

	float* vertices = new float[3 * MAX_TRIANGLES];
	int* indices = new int[3 * MAX_TRIANGLES];
	int currentVertex = 0;
	int currentIndex = 0;

	std::string line;
	while (std::getline(meshFile, line)) {

		// Get the type of line (v/f, but not vn, ...)
		std::string type = line.substr(0, line.find(' '));

		// Get the first, second, and third arguments
		std::istringstream iss(line);
		std::string arg1, arg2, arg3;
		iss >> type >> arg1 >> arg2 >> arg3;

		if (type != "v" && type != "f") {
			continue;
		}

		if (type == "v") {
			vertices[currentVertex++] = std::stof(arg1);
			vertices[currentVertex++] = std::stof(arg2);
			vertices[currentVertex++] = std::stof(arg3);
		}
		else if (type == "f") {
			indices[currentIndex++] = std::stoi(arg1) - 1;
			indices[currentIndex++] = std::stoi(arg2) - 1;
			indices[currentIndex++] = std::stoi(arg3) - 1;
		}

		if (currentVertex > 3 * MAX_TRIANGLES || currentIndex > 3 * MAX_TRIANGLES) {
			delete[] vertices;
			delete[] indices;
			std::cerr << "Error: Too many vertices or indices in mesh file\n";
			std::cerr << "Maximum is " << MAX_TRIANGLES << "\n";
			return false;
		}
	}

	mesh.firstTriangle = objectBuffer.numTriangles;

	for (int i = 0; i < currentIndex; i += 3) {
		if (!createTriangle(objectBuffer, glm::vec3(vertices[3 * (indices[i])], vertices[3 * (indices[i]) + 1], vertices[3 * (indices[i]) + 2]),
			glm::vec3(vertices[3 * (indices[i + 1])], vertices[3 * (indices[i + 1]) + 1], vertices[3 * (indices[i + 1]) + 2]),
			glm::vec3(vertices[3 * (indices[i + 2])], vertices[3 * (indices[i + 2]) + 1], vertices[3 * (indices[i + 2]) + 2]),
			Material{ glm::vec3(1.0f, 1.0f, 1.0f), 0.f, glm::vec4(0.f) }))
		{
			delete[] vertices;
			delete[] indices;
			objectBuffer.numTriangles = mesh.firstTriangle;
			return false;
		}
	}

	printf("Loaded mesh with %d triangles\n", currentIndex / 3);
	printf("Currently %d triangles in total\n", objectBuffer.numTriangles);

	mesh.lastTriangle = objectBuffer.numTriangles - 1;
	mesh.wasLoaded = true;
	mesh.center = glm::vec3(0.f);


	delete[] vertices;
	delete[] indices;

	return true;
}

void scaleMesh(ObjectBuffer& objectBuffer, Mesh& mesh, const glm::vec3& scale, const glm::vec3& center = glm::vec3(0.f)) {

	if (!mesh.wasLoaded) return;

	for (int i = mesh.firstTriangle; i <= mesh.lastTriangle; ++i) {
		objectBuffer.triangles[i].v0 = (objectBuffer.triangles[i].v0 - center) * scale + center;
		objectBuffer.triangles[i].v1 = (objectBuffer.triangles[i].v1 - center) * scale + center;
		objectBuffer.triangles[i].v2 = (objectBuffer.triangles[i].v2 - center) * scale + center;
	}

	mesh.center = (mesh.center - center) * scale + center;
}

void translateMesh(ObjectBuffer& objectBuffer, Mesh& mesh, const glm::vec3& translation) {

	if (!mesh.wasLoaded) return;

	for (int i = mesh.firstTriangle; i <= mesh.lastTriangle; ++i) {
		objectBuffer.triangles[i].v0 += translation;
		objectBuffer.triangles[i].v1 += translation;
		objectBuffer.triangles[i].v2 += translation;
	}

	mesh.center += translation;
}

void rotateMesh(ObjectBuffer& objectBuffer, Mesh& mesh, const float angle, const glm::vec3& axis, const glm::vec3& center = glm::vec3(0.f)) {

	if (!mesh.wasLoaded) return;

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);

	for (int i = mesh.firstTriangle; i <= mesh.lastTriangle; ++i) {
		objectBuffer.triangles[i].v0 = glm::vec3(rotation * glm::vec4(objectBuffer.triangles[i].v0 - center, 1.0f)) + center;
		objectBuffer.triangles[i].v1 = glm::vec3(rotation * glm::vec4(objectBuffer.triangles[i].v1 - center, 1.0f)) + center;
		objectBuffer.triangles[i].v2 = glm::vec3(rotation * glm::vec4(objectBuffer.triangles[i].v2 - center, 1.0f)) + center;
	}

	mesh.center = glm::vec3(rotation * glm::vec4(mesh.center - center, 1.0f)) + center;
}

void setMeshMaterial(ObjectBuffer& objectBuffer, Mesh& mesh, const Material& material) {

	if (!mesh.wasLoaded) return;

	for (int i = mesh.firstTriangle; i <= mesh.lastTriangle; ++i) {
		objectBuffer.triangles[i].material = material;
	}
}

void setMeshColor(ObjectBuffer& objectBuffer, Mesh& mesh, const glm::vec3& color) {

	if (!mesh.wasLoaded) return;

	for (int i = mesh.firstTriangle; i <= mesh.lastTriangle; ++i) {
		objectBuffer.triangles[i].material.color = color;
	}
}

void setMeshSmoothness(ObjectBuffer& objectBuffer, Mesh& mesh, const float smoothness) {

	if (!mesh.wasLoaded) return;

	for (int i = mesh.firstTriangle; i <= mesh.lastTriangle; ++i) {
		objectBuffer.triangles[i].material.smoothness = smoothness;
	}
}

void setMeshEmission(ObjectBuffer& objectBuffer, Mesh& mesh, const glm::vec4& emission) {

	if (!mesh.wasLoaded) return;

	for (int i = mesh.firstTriangle; i <= mesh.lastTriangle; ++i) {
		objectBuffer.triangles[i].material.emission = emission;
	}
}

int getMeshOf(const int ROIndex, const ObjectBuffer& objectBuffer, const Mesh* meshes, const int numMeshes) {

	for (int i = 0; i < numMeshes; ++i) {
		if (meshes[i].firstTriangle <= ROIndex && meshes[i].lastTriangle >= ROIndex) {
			return i;
		}
	}

	return -1;
	
}

bool isTriangle(const int ROIndex, const ObjectBuffer& objectBuffer) {
	return ROIndex >= MAX_SPHERES && ROIndex < MAX_SPHERES + MAX_TRIANGLES;
}

int getSelection(const ObjectBuffer& objectBuffer, Mesh* const meshes, const int numMeshes, const glm::vec2 mousePos) {

	const int clicked = getROIndexAt(mousePos, objectBuffer);

	if (clicked < 0) return -1;

	if (isTriangle(clicked, objectBuffer)) {
		const int meshIndex = getMeshOf(clicked - MAX_SPHERES, objectBuffer, meshes, numMeshes);
		if (meshIndex < 0) return -1;
		return meshIndex + MAX_SPHERES;
	}

	return clicked;
}

void translateObject(ObjectBuffer& objectBuffer, Mesh* const meshes, const int index, const glm::vec3& translation) {

	if (index >= 0) {
		if (index < MAX_SPHERES) {
			objectBuffer.spheres[index].center += translation;
		}
		else {
			translateMesh(objectBuffer, meshes[index - MAX_SPHERES], translation);
		}
	}
}

void setObjectColor(ObjectBuffer& objectBuffer, Mesh* const meshes, const int index, const glm::vec3& color) {

	if (index >= 0) {
		if (index < MAX_SPHERES) {
			objectBuffer.spheres[index].material.color = color;
		}
		else {
			setMeshColor(objectBuffer, meshes[index - MAX_SPHERES], color);
		}
	}
}