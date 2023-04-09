#pragma once

#include <GL/glew.h>

// Compile shader source code and return shader object
bool compileShader(const std::string& shaderSource, GLenum shaderType, GLuint& shader) {
	shader = glCreateShader(shaderType);
	const char* shaderSourceC = shaderSource.c_str();
	glShaderSource(shader, 1, &shaderSourceC, NULL);
	glCompileShader(shader);
	char infoLog[512];
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cerr << "Shader compilation failed: " << infoLog << "\n";
		return false;
	}
	return true;
}

// Link vertex and fragment shaders into a shader program and return program object
bool linkShaderProgram(GLuint vertexShader, GLuint fragmentShader, GLuint& shaderProgram) {
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	char infoLog[512];
	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "Shader program linking failed: " << infoLog << "\n";
		return false;
	}
	return true;
}

bool loadShader(const char* shaderName, GLuint& shaderProgram) {

	// Open shader files
	std::ifstream vertexShaderFile(std::string(shaderName) + ".vert");
	std::ifstream fragmentShaderFile(std::string(shaderName) + ".frag");
	if (!vertexShaderFile.is_open() || !fragmentShaderFile.is_open()) {
		std::cout << "Error: Could not open shader files\n";
		return false;
	}

	// Read shader source code from files
	std::string vertexShaderSource((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());
	std::string fragmentShaderSource((std::istreambuf_iterator<char>(fragmentShaderFile)), std::istreambuf_iterator<char>());

	// Compile and link shaders into a shader program
	GLuint vertexShader, fragmentShader;
	if (!compileShader(vertexShaderSource, GL_VERTEX_SHADER, vertexShader)) return false;
	if (!compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER, fragmentShader)) return false;
	if (!linkShaderProgram(vertexShader, fragmentShader, shaderProgram)) return false;

	// Delete shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Use the shader program
	glUseProgram(shaderProgram);

	return true;

}