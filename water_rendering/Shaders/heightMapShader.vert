#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in mat4 instanceTransform; 

out vec3 fragmentPosition; 
out vec3 normal; 

/*generated end*/

uniform sampler2D heightMap;

void main() {
	vec3 position = vertexPosition;
	float height = texture(heightMap, vertexPosition.xz / 200.0).r;
	height = height * 2.5;
	position.y += height;

	normal = vertexNormal;
	fragmentPosition = position;
	gl_Position = instanceTransform * vec4(position, 1.0);
	//gl_Position = instanceTransform * vec4(vec3(vertexPosition.x, height, vertexPosition.z), 1.0);
}
