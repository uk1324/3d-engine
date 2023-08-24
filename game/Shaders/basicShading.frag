#version 430 core
out vec4 fragColor;

/*generated end*/

in vec3 normal;

void main() {
	// fragColor = vec4(1, 1, 1, 1);
	//fragColor = vec4(gl_FragCoord.zzz, 1);
	//fragColor = vec4(1, 0, 0, 1);
	float d = dot(normalize(normal), vec3(0, -1, 0));
	//fragColor = vec4(vec3(normal), 1);
	//fragColor = vec4(vec3(clamp(d, 0, 1) + 0.2), 1);
	//fragColor = vec4((normal + vec3(1.0)) / 2.0, 1);
	fragColor = vec4((normalize(normal) + vec3(1.0)) / 2.0, 1);
}
