#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in vec3 instanceColor; 

out vec3 color; 

void passToFragment() {
    color = instanceColor; 
}

/*generated end*/

void main() {
    passToFragment();
    gl_Position = instanceTransform * vec4(vertexPosition, 1.0);
}
