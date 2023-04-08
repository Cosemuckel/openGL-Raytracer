#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void createBuffers(ObjectBuffer& objectBuffer, GLuint& VAO, GLuint& VBO, GLuint& EBO, GLuint& UBO, GLuint& UBOIndex, GLuint& shaderProgram) {

	GLfloat vertices[12] = {
	-1.0f, -1.0f, 0.0f,
	 1.0f, -1.0f, 0.0f,
	 1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f, 0.0f
	};

	GLuint indices[6] = {
		0, 1, 2,
		2, 3, 0
	};

	// Generate a vertex array object, a vertex buffer object, and an element buffer object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Bind the vertex array object
	glBindVertexArray(VAO);

	// Bind and fill the vertex buffer object with vertex data
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind and fill the element buffer object with index data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Specify how the vertex buffer object data is laid out and enable the first attribute (position)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Unbind the vertex buffer object
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Unbind the vertex array object
	glBindVertexArray(0);

	// Generate a uniform buffer object
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ObjectBuffer), &objectBuffer, GL_DYNAMIC_DRAW);

	// Get the index of the uniform buffer object in the shader program
	UBOIndex = glGetUniformBlockIndex(shaderProgram, "ObjectBuffer");

	// Set the uniform buffer object to be bound to the binding point 0
	glUniformBlockBinding(shaderProgram, UBOIndex, 0);
}

void freeBuffers(GLuint& VAO, GLuint& VBO, GLuint& EBO, GLuint& UBO) {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &UBO);
}

void initBufferData(ObjectBuffer& objectBuffer, Mesh* const meshes) {
	objectBuffer.resolution = glm::vec2(windowWidth, windowHeight);
	objectBuffer.numSpheres = 0;
	objectBuffer.numTriangles = 0;

	objectBuffer.maxBounces = 5;
	objectBuffer.numSamples = 52;

	objectBuffer.jitterStrenght = .8f;

	//Camera facing forward
	objectBuffer.camera.position = glm::vec3(0.0f, 0.0f, 0.0f);
	objectBuffer.camera.direction = glm::vec3(0.0f, 0.0f, -1.0f);
	objectBuffer.camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
	objectBuffer.camera.right = glm::vec3(1.0f, 0.0f, 0.0f);

}

void initGL(GLFWwindow*& window) {
	// Initialize GLFW library
	glfwInit();

	// Set OpenGL version and profile to use
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Disable window resizing
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// Create window and make it the current OpenGL context
	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set window callbacks
	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetKeyCallback(window, glfwKeyCallback);

	// Initialize GLEW library
	glewExperimental = GL_TRUE;
	glewInit();

	// Set the viewport to the size of the window
	glViewport(0, 0, windowWidth, windowHeight);
}