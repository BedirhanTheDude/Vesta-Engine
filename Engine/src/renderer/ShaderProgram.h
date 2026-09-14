#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <filesystem>

class ShaderProgram {
public:
	ShaderProgram(const std::string& name);
	~ShaderProgram();

	void setBool(const std::string& uniformName, bool value) const;
	void setInt(const std::string& uniformName, int value) const;
	void setFloat(const std::string& uniformName, float value) const;
	void setVec3(const std::string& uniformName, const glm::vec3& value) const;
	void setMat4(const std::string& uniformName, const glm::mat4& value) const;

	template<typename T>
	void setSSBO(GLuint binding, GLuint* SSBO, const std::string& countUniform, const std::vector<T>& buffer) {
		setInt(countUniform, buffer.size());
		
		if (buffer.empty()) {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, 0);
			return;
		}
		
		if (*SSBO == 0)
			glGenBuffers(1, SSBO);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, *SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			buffer.size() * sizeof(T),
			buffer.data(),
			GL_DYNAMIC_DRAW
		);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, *SSBO);
	}

	void use() const;

	std::string getName() const { return name; }
	void setName(const std::string& name) { this->name = name; }
	
	unsigned int getID() const;
private:
	unsigned int programID = 0;

	std::string name;
	std::string readFile(const std::filesystem::path& path);
	unsigned int compileShader(unsigned int type, const std::string& source);
};