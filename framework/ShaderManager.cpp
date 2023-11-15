#include "ShaderManager.hpp"
#include <filesystem>
#include <string_view>
#include <engine/Utils/Put.hpp>

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
	shaderEntries.push_back(ShaderEntry{
		.vertPath = vertPath,
		.vertPathLastWriteTime = std::filesystem::last_write_time(vertPath),
		.fragPath = fragPath,
		.fragPathLastWriteTime = std::filesystem::last_write_time(fragPath),
		.program = ShaderProgram::compile(vertPath, fragPath),
	});
	auto& shader = shaderEntries.back();
 	return shader.program;
}
