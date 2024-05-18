#version 430 core

uniform float time; 
uniform vec2 cameraPosition; 

in vec2 worldPosition; 
in vec2 position; 

in float t; 
out vec4 fragColor;

/*generated end*/

#include "Utils/noise3.glsl"

void main() {
	vec2 p = position;
	p -= 0.5;
	p *= 2.0;

    vec3 col;

    p *= 3.0;

    col = (vec3(p, 0.0));
    const int vertexCount = 5;
    vec3 vertices[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
        float t = float(i) / float(vertexCount);
        float a = t * 2.0 * 3.141592;
        vertices[i] = vec3(cos(a), sin(a), 1.0);
    }
 
    float d;


    vec3 colr;
    float t1 = 0.005;
    float t2 = 0.11;

    vec3 c1 = vec3(1./4., 1.0 ,1./16.);
    vec3 c2 = vec3(1.0 / 4.0);
    vec3 c3 = c1 * 3;

    d = length(p);

    if (t < t1) {
        float lt = smoothstep(0.0, t1, t);
        colr = mix(c1, c3, lt);
        d -= lt * 0.1;
    } else if (t < t2) {
        float lt = smoothstep(t1, t2, t);
        colr = mix(c3, c2, lt);
        d -= (1.0 - lt) * 0.1;
    } else {
        colr = mix(c2, c1, smoothstep(t2, 1.0, t));
    }

    d += octave01(vec3(p, time * 2.0), 4) * 0.5;
    d = smoothstep(1.0, 0.25, d);

    col = vec3(d);
    float g = octave01(vec3(p * 4.0, time), 4);




    vec3 c = colr * exp(4.*g - 1.);
 
    col *= c / 2.0;
    col /= 4.0;

    fragColor = vec4(col, d);
}
