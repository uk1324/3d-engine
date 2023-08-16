#version 430 core
out vec4 fragColor;

/*generated end*/

in vec3 position;

void main() {
	fragColor = vec4(position, 1);
}
