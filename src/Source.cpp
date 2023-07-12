#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/scalar_multiplication.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <unordered_map>

/*
	Small docs:
		- `render object` (`RO`) refers to spheres and triangles
		- `scene object` (`SO`) refers to spheres and meshes
		- When working with objects, we usually work with scene objects, except when explicitly stated otherwise (e.g. `getROIndexAt` will return the index of the object in the object buffer)
		- If the index points to a sphere, it does not matter whether it is a scene object or a render object
*/

int windowWidth = 800, windowHeight = 600;
constexpr int MAX_SPHERES = 2;
constexpr int MAX_TRIANGLES = 30;

constexpr int MAX_BOUNCES = 2;
constexpr int NUM_SAMPLES = 100;

constexpr glm::vec2  p720 = glm::vec2(1280, 720);
constexpr glm::vec2  p1080 = glm::vec2(1920, 1080);
constexpr glm::vec2  p1440 = glm::vec2(2560, 1440);
constexpr glm::vec2  p4k = glm::vec2(3840, 2160);
constexpr glm::vec2 p8k = glm::vec2(7680, 4320);

void glfwErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << "\n";
}

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	windowWidth = width;
	windowHeight = height;
}

#include "Structures.h"
#include "Shader.h"
#include "Initialization.h"
#include "Selection.h"
#include "Bodies.h"
#include "Gui.h"

void updateCamera() {
	

}

void updateScene(ObjectBuffer& objectBuffer, Mesh* const meshes, int numMeshes) {

	if (!numMeshes) return;
	
	rotateMesh(objectBuffer, meshes[0], 0.01f, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -3.0f));
}

void processKeyboardInput(GLFWwindow* const window, ObjectBuffer& objectBuffer, Mesh* const meshes, const int selectedObject) {
	static constexpr float moveSpeed = 0.03f;

	static const std::unordered_map<int, glm::vec3> keyMap = {
		{ GLFW_KEY_LEFT,  glm::vec3(-1.0f,  0.0f,  0.0f) },
		{ GLFW_KEY_RIGHT, glm::vec3(1.0f,  0.0f,  0.0f) },
		{ GLFW_KEY_UP,    glm::vec3(0.0f,  1.0f,  0.0f) },
		{ GLFW_KEY_DOWN,  glm::vec3(0.0f, -1.0f,  0.0f) },
		{ GLFW_KEY_S,     glm::vec3(0.0f,  0.0f,  1.0f) },
		{ GLFW_KEY_W,     glm::vec3(0.0f,  0.0f, -1.0f) }
	};

	for (const auto& [key, direction] : keyMap)
		if (glfwGetKey(window, key) == GLFW_PRESS)
			translateObject(objectBuffer, meshes, selectedObject, direction * moveSpeed);
}

void update(GLFWwindow* const window, ObjectBuffer& objectBuffer, Mesh* const meshes, const int numMeshes) {
	static bool isFirstMousePress = true;
	static int selectedObject = -1; // -1 = no object selected, 0 -> first sphere, until MAX_SPHERES, then meshes

	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		glm::vec3 color;
		bool colorSelected = selectColor(mouseX, mouseY, color);

		if (!colorSelected && isFirstMousePress) {
			selectedObject = getSelection(objectBuffer, meshes, numMeshes, (glm::vec2(mouseX, mouseY) - glm::vec2(windowWidth / 2, windowHeight / 2)) / windowHeight);
		}

		if (colorSelected && selectedObject >= 0) {
			if (selectedObject < MAX_SPHERES) {
				objectBuffer.spheres[selectedObject].material.color = color;
			}
			else {
				setMeshColor(objectBuffer, meshes[selectedObject - MAX_SPHERES], color);
			}
		}

		isFirstMousePress = false;
	}
	else {
		isFirstMousePress = true;
	}

	if (selectedObject >= 0) {

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			if (selectedObject < MAX_SPHERES)
				objectBuffer.spheres[selectedObject].center += glm::vec3(0.0f, 0.0f, 0.1f);
			else
				translateMesh(objectBuffer, meshes[selectedObject - MAX_SPHERES], glm::vec3(0.0f, 0.0f, 0.1f));
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			if (selectedObject < MAX_SPHERES)
				objectBuffer.spheres[selectedObject].center += glm::vec3(0.0f, 0.0f, -0.1f);
			else
				translateMesh(objectBuffer, meshes[selectedObject - MAX_SPHERES], glm::vec3(0.0f, 0.0f, -0.1f));
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			if (selectedObject < MAX_SPHERES)
				objectBuffer.spheres[selectedObject].center += glm::vec3(-0.1f, 0.0f, 0.0f);
			else
				translateMesh(objectBuffer, meshes[selectedObject - MAX_SPHERES], glm::vec3(-0.1f, 0.0f, 0.0f));
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			if (selectedObject < MAX_SPHERES)
				objectBuffer.spheres[selectedObject].center += glm::vec3(0.1f, 0.0f, 0.0f);
			else
				translateMesh(objectBuffer, meshes[selectedObject - MAX_SPHERES], glm::vec3(0.1f, 0.0f, 0.0f));
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			if (selectedObject < MAX_SPHERES)
				objectBuffer.spheres[selectedObject].center += glm::vec3(0.0f, 0.1f, 0.0f);
			else
				translateMesh(objectBuffer, meshes[selectedObject - MAX_SPHERES], glm::vec3(0.0f, 0.1f, 0.0f));
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			if (selectedObject < MAX_SPHERES)
				objectBuffer.spheres[selectedObject].center += glm::vec3(0.0f, -0.1f, 0.0f);
			else
				translateMesh(objectBuffer, meshes[selectedObject - MAX_SPHERES], glm::vec3(0.0f, -0.1f, 0.0f));

	}

	updateCamera();
	updateScene(objectBuffer, meshes, numMeshes);
	computeTriangles(objectBuffer);

	objectBuffer.resolution = glm::vec2(windowWidth, windowHeight);
}


void render(GLFWwindow* window, ObjectBuffer& objectBuffer, GLuint& VAO, GLuint& UBO, GLuint& UBOIndex, GLuint& shaderProgram) {
	// Clear the screen buffer
	glClear(GL_COLOR_BUFFER_BIT);

	// Pass data to the uniform buffer object
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ObjectBuffer), &objectBuffer);

	// Draw the elements using the bound buffers and shader program
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		
	// Swap the back and front buffers to display the rendered frame
	glfwSwapBuffers(window);
}

void exportRender(ObjectBuffer& objectBuffer, GLuint& VAO, GLuint& UBO, GLuint& UBOIndex, GLuint& shaderProgram, int numSamples, int maxBounces, glm::u16vec2 resolution) {
	
	//In order to export the render, we need to create a new frame buffer and texture to render to
	GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	
	//We now check if the frame buffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error: Frame buffer is not complete!\n";
		return;
	}

	//We now set the sample and bounce count, but also save the old values so we can reset them later
	int oldNumSamples = objectBuffer.numSamples;
	int oldMaxBounces = objectBuffer.maxBounces;
	objectBuffer.numSamples = numSamples;
	objectBuffer.maxBounces = maxBounces;

	//Set the resolution of the render
	glViewport(0, 0, resolution.x, resolution.y);
	objectBuffer.resolution = glm::vec2(resolution.x, resolution.y);
	objectBuffer.jitterStrenght *= windowWidth;
	objectBuffer.jitterStrenght /= resolution.x;
		
	//We now need to do the usual rendering process
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ObjectBuffer), &objectBuffer);
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	//We now need to read the data from the texture and write it to a file
	unsigned char* data = new unsigned char[resolution.x * resolution.y * 3];
	glReadPixels(0, 0, resolution.x, resolution.y, GL_RGB, GL_UNSIGNED_BYTE, data);
	
	std::string filename = "render_" + std::to_string(numSamples) + "_" + std::to_string(maxBounces) + "__" + std::to_string(resolution.x) + "x" + std::to_string(resolution.y) + ".png";
	stbi_flip_vertically_on_write(true);
	stbi_write_png(filename.c_str(), resolution.x, resolution.y, 3, data, resolution.x * 3);

	std::cout << "Render exported to " << filename << std::endl;
	
	delete[] data;

	//We now need to reset the sample and bounce count
	objectBuffer.numSamples = oldNumSamples;
	objectBuffer.maxBounces = oldMaxBounces;

	//We now need to reset the resolution
	glViewport(0, 0, windowWidth, windowHeight);
	objectBuffer.resolution = glm::vec2(windowWidth, windowHeight);
	objectBuffer.jitterStrenght *= resolution.x;
	objectBuffer.jitterStrenght /= windowWidth;
	
	//We now need to delete the frame buffer and texture
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteTextures(1, &texture);
		
}

int main() {
	
	GLFWwindow* window = nullptr;	

	GLuint VBO, VAO, EBO;
	GLuint UBO, UBOIndex; // Uniform Buffer Object to pass data to the shader

	GLuint shaderTraceProgram;

	Mesh meshes[2];
	int numMeshes = 0;
	ObjectBuffer objectBuffer;
	
	initGL(window);

	if (!loadShader("shaders/trace", shaderTraceProgram)) return -1;
	createBuffers(objectBuffer, VAO, VBO, EBO, UBO, UBOIndex, shaderTraceProgram);

	initBufferData(objectBuffer, meshes);
	
	createSphere(objectBuffer, glm::vec3(15.0f, 15.0f, .0f), 20.0f, Material{ glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, glm::vec3(1.0f) });
	createSphere(objectBuffer, glm::vec3(-1.5f, .2f, -4.0f), 1.0f, Material{ glm::vec3(1.0f, 1.0f, 0.0f), 0.2f, glm::vec3(0.0f) });
	
	if (loadMesh(objectBuffer, "meshes/ico_sphere.obj", meshes[0])) {
		setMeshMaterial(objectBuffer, meshes[0], Material{ glm::vec3(1.0f, 0.0f, 1.0f), .3f, glm::vec3(0.0f) });
		translateMesh(objectBuffer, meshes[0], glm::vec3(0.0f, 0.0f, -3.0f));
		numMeshes++;
	}
	
	if (loadMesh(objectBuffer, "meshes/plane.obj", meshes[1])) {
		setMeshMaterial(objectBuffer, meshes[1], Material{ glm::vec3(1.0f, 1.0f, 1.0f), 0.f, glm::vec3(0.0f) });
		translateMesh(objectBuffer, meshes[1], glm::vec3(0.0f, -.5f, -3.0f));
		numMeshes++;
	}

	// Set the clear color for the screen
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	computeTriangles(objectBuffer);
	exportRender(objectBuffer, VAO, UBO, UBOIndex, shaderTraceProgram, 1000, 2, p8k);

	unsigned int frames = 0;

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

	// Main loop
	while (!glfwWindowShouldClose(window)) {

		// Check for input events
		glfwPollEvents();

		update(window, objectBuffer, meshes, numMeshes);

		// Render the scene
		render(window, objectBuffer, VAO, UBO, UBOIndex, shaderTraceProgram);

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