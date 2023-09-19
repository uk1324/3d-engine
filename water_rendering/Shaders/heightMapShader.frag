#version 430 core

in vec3 fragmentPosition; 
in vec3 normal; 
out vec4 fragColor;

/*generated end*/

uniform sampler2D heightMap;

void main() {
	//float height = texture(heightMap, fragmentPosition.xz / 100.0).r;
	float height = texture(heightMap, fragmentPosition.xz / 100.0).r;
	//fragColor = texture(heightMap, fragmentPosition.xz / 100.0 * 5.0);
	fragColor = vec4(vec3(abs(height) / 256), 1.0);
	//fragColor = vec4(fragmentPosition.xz / 100.0 * 5.0, 0.0, 1.0);
}
