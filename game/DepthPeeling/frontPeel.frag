#version 430 core
out vec4 fragColor;

/*generated end*/

uniform sampler2DRect depthTexture;

in vec3 normal;

void main() {
	float frontDepth = texture(depthTexture, gl_FragCoord.xy).r;
	//fragColor = vec4(vec3(frontDepth), 1);
	if (gl_FragCoord.z <= frontDepth) {
		discard;
	}
	
	//vec4 color = vec4((normal + vec3(1.0)) / 2.0, 0.23);
	//vec4 color = vec4(vec3(0.2), 0.23);
	
	vec4 color = vec4(vec3(clamp(abs(normal.z), 0.2, 1.0)), 0.23);
	//vec4 color = vec4((normalize(normal) + vec3(1.0)) / 2.0, 0.2);
	//fragColor = vec4(color.rgb * color.a, color.a);
	fragColor = color;
}



