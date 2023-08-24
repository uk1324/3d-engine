#version 430 core

uniform mat4 inverseViewProjection; 
uniform vec2 screenSize; 
out vec4 fragColor;

/*generated end*/

#include "Utils/posFromDepth.glsl"
#include "Utils/sampleFiltered.glsl"

vec3 sampleTexture(vec2 p) {
	ivec2 gridPos = ivec2(floor(p));

	if (gridPos.x % 2 == gridPos.y % 2) {
		return vec3(1);
	} else {
		return vec3(0);
	}
}

GENERATE_SAMPLE_FILTERED(sampleTextureFiltered, sampleTexture)

void main() {
	vec3 worldPos = posFromDepth(gl_FragCoord.xy / screenSize, gl_FragCoord.z, inverseViewProjection);
	
	vec2 p = worldPos.xz;
    vec3 color;
    color = sampleTextureFiltered(p, dFdx(p), dFdy(p), 4) * 0.3;

	fragColor = vec4(color, 1);
}
