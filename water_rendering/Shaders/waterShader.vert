#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 

uniform float time; 

out vec3 unnormalizedNormal; 
out vec3 fragmentWorldPosition; 

/*generated end*/

float hash21(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

float hash11(float p) {
	vec3 p3  = fract(vec3(p) * 0.1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

float getHeight(vec2 p) {
	//p += vec2(time, 0.0);
	p /= 10.0;

	float h = 0.0;

	float scale = 0.76;
	{
		float amplitude = 0.5;
		float frequency = 1.0;
		for (int i = 0; i < 5; i++) {
			float angle = hash11(i);
			float d = dot(vec2(cos(angle), sin(angle)), p);
			h += amplitude * sin(frequency * d + time / 3.0);
			amplitude /= 2.0;
			frequency *= 2.0;
		}
	}

	h /= 2.0;
	h *= 15.0;

	return h;
}

void main() {
	vec3 v0 = vec3(0.0, getHeight(vertexPosition), 0.0);
	float stp = 1.0;
	vec3 v1 = vec3(stp, getHeight(vec2(vertexPosition.x + stp, vertexPosition.y)), 0.0);
	vec3 v2 = vec3(0.0, getHeight(vec2(vertexPosition.x, vertexPosition.y + stp)), stp);
	// After interpolation it has to be normalized so it isn't normalized here.

	unnormalizedNormal = cross(v1 - v0, v2 - v0);
	fragmentWorldPosition = vec3(vertexPosition.x, v0.y, vertexPosition.y);
	gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);

	{
		vec2 p = vertexPosition;
		vec2 h = vec2(0.1, 0.0);
		unnormalizedNormal = normalize(vec3(
			getHeight(p-h.xy) - getHeight(p+h.xy),
            2.0 * h.x,
            getHeight(p-h.yx) - getHeight(p+h.yx)
		));
	}
}
