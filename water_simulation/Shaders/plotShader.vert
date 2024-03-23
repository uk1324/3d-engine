#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in mat4 instanceModel; 
layout(location = 9) in vec3 instanceScale; 
layout(location = 10) in vec2 instanceSamplingScale; 
layout(location = 11) in float instanceColormapMin; 
layout(location = 12) in float instanceColormapMax; 

out vec3 fragNormal; 
out float colormapValue01; 
out vec3 fragmentWorldPosition; 

/*generated end*/

uniform sampler2D heightmap;

vec2 pixelSize = 1.0 / vec2(textureSize(heightmap, 0));

// [0, 1]
vec2 texturePos = vec2(vertexPosition.x, vertexPosition.z) * instanceSamplingScale * (vec2(1) - 2 * pixelSize) + pixelSize;
// Adding and scaling by pixelsize is a hacky way to prevent clamped points being used in normal calculations.

float height(vec2 p) {
	return texture(heightmap, p).x * instanceScale.y;
}

float normalSample(vec2 p) {
	return height(p / instanceScale.xz);
}

vec3 getNormal(const vec2 p)  {
	// [0, scaledRangeSize] so the normals are calculated correctly regardless of the scaling.
	vec2 pos = p * instanceScale.xz;

    const float eps = 0.01;
    const vec2 h = vec2(eps, 0);
    return normalize(vec3(
		normalSample(pos - h.xy).x - normalSample(pos + h.xy).x,
		2.0 * h.x,
		normalSample(pos - h.yx).x - normalSample(pos + h.yx).x
	)); 
}

void main() {
	float y = height(texturePos);
	vec3 vertex = vec3(vertexPosition.x, y, vertexPosition.z);
	gl_Position = instanceTransform * vec4(vertex, 1.0);
	vertex.y /= instanceScale.y;
	fragmentWorldPosition = (instanceModel * vec4(vertex, 1.0)).xyz;
	//normal = vec3(texturePos, 0.0);
	fragNormal = getNormal(texturePos);
	colormapValue01 = (y - instanceColormapMin) / (instanceColormapMax - instanceColormapMin);
	//normal = vec3(1, 0, 0);
}
