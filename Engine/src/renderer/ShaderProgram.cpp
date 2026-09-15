#include "ShaderProgram.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

ShaderProgram::ShaderProgram(const std::string& name) {
	this->name = name;

	std::filesystem::path shaderBase = std::filesystem::current_path() / "assets" / "shaders";
	std::string vertexSource = readFile(shaderBase / (name + ".vert"));
	std::string fragmentSource = readFile(shaderBase / (name + ".frag"));

	unsigned int vertexShader   = compileShader(GL_VERTEX_SHADER, vertexSource);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

	programID = glCreateProgram();
	glAttachShader(programID, vertexShader);
	glAttachShader(programID, fragmentShader);
	glLinkProgram(programID);

	int success = 0;
	char infoLog[512];

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, 512, nullptr, infoLog);
		std::cerr << "ERROR: Failed to link shader program\n" << infoLog << '\n';
		throw std::runtime_error("Failed to link shader program");
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

ShaderProgram::~ShaderProgram() {
	if (programID != 0) {
		glDeleteProgram(programID);
	}
}

void ShaderProgram::use() const {
	glUseProgram(programID);
}

void ShaderProgram::setBool(const std::string& uniformName, bool value) const {
	int location = glGetUniformLocation(programID, uniformName.c_str());
	glUniform1i(location, (int)value);
}

void ShaderProgram::setInt(const std::string& uniformName, int value) const {
	int location = glGetUniformLocation(programID, uniformName.c_str());
	glUniform1i(location, value);
}

void ShaderProgram::setFloat(const std::string& uniformName, float value) const {
	int location = glGetUniformLocation(programID, uniformName.c_str());
	glUniform1f(location, value);
}

void ShaderProgram::setVec3(const std::string& uniformName, const glm::vec3& value) const {
	int location = glGetUniformLocation(programID, uniformName.c_str());
	glUniform3fv(location, 1, glm::value_ptr(value));
}

void ShaderProgram::setMat4(const std::string& uniformName, const glm::mat4& value) const {
	int location = glGetUniformLocation(programID, uniformName.c_str());
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

unsigned int ShaderProgram::getID() const {
	return programID;
}

std::string ShaderProgram::readFile(const std::filesystem::path& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "ERROR: Failed to open shader file: " << path << '\n';
		throw std::runtime_error("Failed to open shader file");
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

unsigned int ShaderProgram::compileShader(unsigned int type, const std::string& source) {
	unsigned int shader = glCreateShader(type);
	const char* src = source.c_str();

	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);

	int success = 0;
	char infoLog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);

		std::cerr << "ERROR: " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
			<< " shader compilation failed:\n"
			<< infoLog << '\n';
		throw std::runtime_error("Shader compilation failed");
	}

	return shader;
}