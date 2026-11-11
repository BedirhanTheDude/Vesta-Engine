#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <string_view>
#include <cstdint>
#include <vector>
#include <deque>
#include <unordered_map>
#include <filesystem>

class ShaderProgram {
public:
	// basePath is the shader's .vert/.frag pair without the extension
	ShaderProgram(const std::string& name, const std::filesystem::path& basePath);
	~ShaderProgram();

	// directory of the built-in shaders the renderer itself uses (lit, depth, ...), lives in the engine source tree
	static std::filesystem::path getEngineShaderDirectory();

	// owns the GL program and views into its own uniform name storage
	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	void setBool(std::string_view uniformName, bool value) const;
	void setInt(std::string_view uniformName, int value) const;
	void setFloat(std::string_view uniformName, float value) const;
	void setVec3(std::string_view uniformName, const glm::vec3& value) const;
	void setMat4(std::string_view uniformName, const glm::mat4& value) const;

	template<typename T>
	void setSSBO(GLuint binding, GLuint* SSBO, std::string_view countUniform, const std::vector<T>& buffer) {
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

	// true the first time it is called with a given epoch (a frame counter), false after that.
	bool beginEpoch(uint64_t epoch) const {
		if (lastEpoch == epoch) return false;

		lastEpoch = epoch;
		return true;
	}

	std::string getName() const { return name; }
	void setName(const std::string& name) { this->name = name; }
	
	unsigned int getID() const;
private:
	int getUniformLocation(std::string_view uniformName) const;

	unsigned int programID = 0;
	mutable uint64_t lastEpoch = 0;

	mutable std::deque<std::string> uniformNames;
	mutable std::unordered_map<std::string_view, int> uniformLocations;

	std::string name;
	std::string readFile(const std::filesystem::path& path);
	unsigned int compileShader(unsigned int type, const std::string& source);
};