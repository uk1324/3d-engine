#pragma once

#include <engine/Graphics/ShaderProgram.hpp>

struct ShaderManager {
	static void update();
	// could use a typedef so this doesn't have to be a reference in release mode. 
	static ShaderProgram& makeShader(const char* vertPath, const char* fragPath);
};

#define MAKE_GENERATED_SHADER(name) ShaderManager::makeShader(name##_SHADER_VERT_PATH, name##_SHADER_FRAG_PATH)