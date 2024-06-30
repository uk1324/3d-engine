#include "ShaderManager.hpp"
#include <filesystem>
#include <string_view>
#include <engine/Utils/Put.hpp>
#include <engine/Utils/Unwrap.hpp>

struct ShaderEntry {
	std::string_view vertPath;
	std::filesystem::file_time_type vertPathLastWriteTime;
	std::string_view fragPath;
	std::filesystem::file_time_type fragPathLastWriteTime;
	ShaderProgram program;
	i32 unsuccessfulReloadTries = 0;

	void tryReload() {
		auto result = ShaderProgram::tryCompile(vertPath, fragPath);
		if (result.has_value()) {
			put("reloaded shader\n"
				"vert: %\n"
				"frag: %",
				vertPath, fragPath
			);
			program = std::move(*result);
		} else {
			put("\a");
			put("tried to reload shader\n"
				"vert: %\n"
				"frag: %\n"
				"%",
				vertPath, fragPath, result.error()
			);
		}
	}
};

// To prevent pointers from invalidating.
std::list<ShaderEntry> shaderEntries;
std::list<ShaderProgram> nonReloadableShaders;

void ShaderManager::update() {
	for (auto& shader : shaderEntries) {
		std::filesystem::file_time_type vertLastWriteTime;
		std::filesystem::file_time_type fragLastWriteTime;
		try {
			vertLastWriteTime = std::filesystem::last_write_time(shader.vertPath);
			fragLastWriteTime = std::filesystem::last_write_time(shader.fragPath);
			shader.unsuccessfulReloadTries = 0;
		} catch (const std::filesystem::filesystem_error&) {
			// Probably caused by the file being written to at the time.
			shader.unsuccessfulReloadTries += 1;
			if (shader.unsuccessfulReloadTries > 20) {
				ASSERT_NOT_REACHED();
			}
			continue;
		}

		if (shader.vertPathLastWriteTime == vertLastWriteTime && shader.fragPathLastWriteTime == fragLastWriteTime) {
			continue;
		}
		shader.tryReload();
		shader.vertPathLastWriteTime = vertLastWriteTime;
		shader.fragPathLastWriteTime = fragLastWriteTime;
	}
}

ShaderProgram& ShaderManager::makeShader(const char* vertPath, const char* fragPath) {
	std::filesystem::file_time_type vertLastWriteTime;
	std::filesystem::file_time_type fragLastWriteTime;
	i32 retryCount = 0;

	std::error_code ec;
	for (;;) {
		try {
			vertLastWriteTime = std::filesystem::last_write_time(vertPath);
			fragLastWriteTime = std::filesystem::last_write_time(fragPath);
			break;
		} catch (const std::filesystem::filesystem_error&) {
			retryCount++;
			if (retryCount > 20) {
				put("error message: %", ec.message());
				ASSERT_NOT_REACHED();
			}
		}
	}

	if (retryCount > 0) {
		//put("retryCount: %" retryCount);
	}

	shaderEntries.push_back(ShaderEntry{
		.vertPath = vertPath,
		.vertPathLastWriteTime = vertLastWriteTime,
		.fragPath = fragPath,
		.fragPathLastWriteTime = fragLastWriteTime,
		.program = ShaderProgram::compile(vertPath, fragPath),
	});
	auto& shader = shaderEntries.back();
 	return shader.program;
}

ShaderProgram& ShaderManager::makeShaderFromSource(const char* vertSource, const char* fragSource) {
	auto shader = unwrap(ShaderProgram::fromSource(vertSource, fragSource));
	nonReloadableShaders.push_back(std::move(shader));
	return nonReloadableShaders.back();
}