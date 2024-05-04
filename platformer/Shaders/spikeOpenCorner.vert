#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 
layout(location = 2) in mat3x2 instanceTransform; 
layout(location = 5) in float instanceRotation; 

uniform mat3x2 clipToWorld; 

out vec2 position; 
out vec2 worldPosition; 

out float rotation; 

void passToFragment() {
    rotation = instanceRotation; 
}

/*generated end*/

void main() {
    passToFragment();
    gl_Position = vec4(instanceTransform * vec3(vertexPosition, 1.0), 0.0, 1.0);
//    position = vertexTexturePosition;
//    position -= vec2(0.5);
//    position *= 2.0;
    position = vertexTexturePosition;
    
    worldPosition = (gl_Position.xy - vec2(0.5)) * 2.0;
	worldPosition = clipToWorld * vec3(worldPosition, 1.0);
}
