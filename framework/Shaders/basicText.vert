#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 
layout(location = 2) in mat3x2 instanceTransform; 
layout(location = 5) in vec2 instanceOffsetInAtlas; 
layout(location = 6) in vec2 instanceSizeInAtlas; 
layout(location = 7) in float instanceSmoothing; 
layout(location = 8) in vec4 instanceColor; 
layout(location = 9) in float instanceOffset; 

out vec2 texturePosition; 
out vec2 atlasMin; 
out vec2 atlasMax; 

out float smoothing; 
out vec4 color; 
out float offset; 

void passToFragment() {
    smoothing = instanceSmoothing; 
    color = instanceColor; 
    offset = instanceOffset; 
}

/*generated end*/

void main() {
	passToFragment();
	texturePosition = vertexTexturePosition;
	texturePosition.y = 1.0 - texturePosition.y;
	texturePosition *= instanceSizeInAtlas;
	texturePosition += instanceOffsetInAtlas;
	atlasMin = instanceOffsetInAtlas;
	atlasMax = atlasMin + instanceSizeInAtlas;
	gl_Position = vec4(instanceTransform * vec3(vertexPosition, 1.0), 0.0, 1.0);
}
