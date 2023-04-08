#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>

constexpr int windowWidth = 800, windowHeight = 600;
constexpr int MAX_SPHERES = 10;
constexpr int MAX_TRIANGLES = 100;

void glfwErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << "\n";
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

#include "Structures.h"
#include "Shader.h"
#include "Initialization.h"
#include "Bodies.h"

void updateCamera() {
	

}

void updateScene(ObjectBuffer& objectBuffer, Mesh* const meshes) {

	rotateMesh(objectBuffer, meshes[0], 0.01f, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -3.0f));
}

void update(ObjectBuffer& objectBuffer, Mesh* const meshes) {
	
	updateCamera();
	updateScene(objectBuffer, meshes);
	computeTriangles(objectBuffer);

}

void render(GLFWwindow* window, ObjectBuffer& objectBuffer, GLuint& VAO, GLuint& UBO, GLuint& UBOIndex, GLuint& shaderProgram) {
	// Clear the screen buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Use the shader program
	glUseProgram(shaderProgram);
	
	// Bind the vertex array object
	glBindVertexArray(VAO);

	// Pass data to the uniform buffer object
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ObjectBuffer), &objectBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, UBOIndex, UBO);

	// Draw the elements using the bound buffers and shader program
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// Unbind the vertex array object
	glBindVertexArray(0);
	
	// Swap the back and front buffers to display the rendered frame
	glfwSwapBuffers(window);
}

int main() {
	
	GLFWwindow* window = nullptr;	

	GLuint VBO, VAO, EBO;
	GLuint UBO, UBOIndex; // Uniform Buffer Object to pass data to the shader

	GLuint shaderProgram;

	Mesh meshes[2];
	ObjectBuffer objectBuffer;
	
	initGL(window);

	loadShader("basic", shaderProgram);
	createBuffers(objectBuffer, VAO, VBO, EBO, UBO, UBOIndex, shaderProgram);

	initBufferData(objectBuffer, meshes);
	
	loadMesh(objectBuffer, "meshes/uv_sphere.obj", meshes[0]);
	setMeshMaterial(objectBuffer, meshes[0], Material{ glm::vec3(1.0f, 0.0f, 1.0f), 0.3f, glm::vec4(0.0f) });
	translateMesh(objectBuffer, meshes[0], glm::vec3(0.0f, 0.0f, -3.0f));
	
	createSphere(objectBuffer, glm::vec3(15.0f, 15.0f, .0f), 20.0f, Material{ glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, glm::vec4(1.0f) });

	// Set the clear color for the screen
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	unsigned int frames = 0;

	// Main loop
	while (!glfwWindowShouldClose(window)) {

		// Check for input events
		glfwPollEvents();

		update(objectBuffer, meshes);

		// Render the scene
		render(window, objectBuffer, VAO, UBO, UBOIndex, shaderProgram);

		++frames;
	}
	
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> time_span = t2 - t1;
	std::cout << "Executed " << frames << " frames in " << time_span.count() / 1000.0 << " seconds.\n";
	std::cout << "Average FPS: " << frames / (time_span.count() / 1000.0) << "\n";
	

	// Clean up
	freeBuffers(VAO, VBO, EBO, UBO);
	glfwTerminate();
	
	
	return 0;
}