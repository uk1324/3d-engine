#version 430 core
out vec4 fragColor;

/*generated end*/

in vec3 normal;

uniform sampler2D depthTexture0;
uniform vec2 screenSize;

void main() {
	//fragColor = vec4(texture2D(depthTexture0, gl_FragCoord.xy / screenSize).rgb, 1);
	////fragColor = vec4(gl_FragCoord.xy / screenSize, 0, 1);
	//return;

	if (gl_FragCoord.z >= texture(depthTexture0, gl_FragCoord.xy / screenSize).r) {
		discard;
	}

	//discard;
	//fragColor = vec4(1, 0, 0, 0.5);
	//fragColor = vec4(vec3(dot(normal, vec3(0, -1, 0))), 0.5);
	//fragColor = vec4(vec3(clamp(dot(normal, vec3(0, -1, 0)), 0.2, 1.0)), 0.1);

	//vec4 color = vec4((normal + vec3(1.0)) / 2.0, 0.23);
	//vec4 color = vec4(vec3(0.2), 0.23);
	vec4 color = vec4(vec3(clamp(abs(normal.z), 0.2, 1.0)), 0.23);
	//vec4 color = vec4((normalize(normal) + vec3(1.0)) / 2.0, 0.2);

	//fragColor = vec4(color.rgb * color.a, 1.0 - color.a);
	//fragColor = vec4(color.rgb * color.a, 1.0 - color.a);
	fragColor = vec4(color.rgb * color.a, 1.0 - color.a);
	//fragColor = color;
}
