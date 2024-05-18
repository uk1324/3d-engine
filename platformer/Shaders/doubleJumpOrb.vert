#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 
layout(location = 2) in mat3x2 instanceTransform; 
layout(location = 5) in float instanceT; 

uniform mat3x2 clipToWorld; 

out vec2 worldPosition; 
out vec2 position; 

out float t; 

void passToFragment() {
    t = instanceT; 
}

/*generated end*/

void main() {
    passToFragment();
    gl_Position = vec4(instanceTransform * vec3(vertexPosition, 1.0), 0.0, 1.0);
    worldPosition = clipToWorld * vec3(gl_Position.xy, 1.0);
    position = vertexTexturePosition;
}
