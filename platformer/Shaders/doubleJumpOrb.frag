#version 430 core

uniform float time; 
uniform vec2 cameraPosition; 

in vec2 position; 

in float t; 
in vec2 orbWorldPosition; 
out vec4 fragColor;

/*generated end*/

#include "Utils/noise3.glsl"

void main() {
	vec2 p = position;
	p -= 0.5;

    vec3 col;

    p *= 3.0;

    float d;

    vec3 colr;

    vec3 c1 = vec3(1./4., 1.0 ,1./16.);
    vec3 c2 = vec3(1.0 / 4.0);
    vec3 c3 = c1 * 3;

    d = length(p);

    float t1 = 0.005;
    float t2 = 0.11;
    float expansion = 0.1;

    if (t < t1) { // Expand and make brighter
        float lt = smoothstep(0.0, t1, t);
        colr = mix(c1, c3, lt);
        d -= lt * expansion;
    } else if (t < t2) { // Contract and make gray
        float lt = smoothstep(t1, t2, t);
        colr = mix(c3, c2, lt);
        d -= (1.0 - lt) * expansion;
    } else { // Go back to idle 
        colr = mix(c2, c1, smoothstep(t2, 1.0, t));
    }

    d += octave01(vec3(p + orbWorldPosition, time * 2.0), 4) * 0.5;
    d = smoothstep(1.0, 0.25, d);

    col = vec3(d);
    float g = octave01(vec3(p * 4.0 + orbWorldPosition, time), 4);

    vec3 c = colr * exp(4.0 * g - 1.0);
 
    col *= c / 2.0;
    col /= 4.0;

    fragColor = vec4(col, d);
}
