#version 430 core

uniform vec3 c0; 
uniform vec3 c1; 

in vec3 unnormalizedDirection; 
out vec4 fragColor;

/*generated end*/

void main() {
	vec3 direction = normalize(unnormalizedDirection);
	vec3 color;
	if (direction.y > 0.0) {
		float angle = atan(direction.y, length(vec2(direction.x, direction.z)));
		color = mix(c0, c1, angle);
	}

	fragColor = vec4(color, 1.0);
}
