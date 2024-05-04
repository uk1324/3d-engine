#version 430 core

in vec2 position; 
in vec2 worldPosition; 

in float time; 
in vec2 normal; 
out vec4 fragColor;

/*generated end*/

#include "spikeShared.glsl"

void main() {

    vec3 col;
    //float d = 1.0 - position.x;
    float d = position.x;
//    d += octave01(vec2(time / 2.0, worldPosition.y / 200.0), 3) * 0.3 - 0.08;
//    d *= 2.5 * 2.5;
//    d = 1.0 / (d  * d);
//    col = vec3(d);
//    vec3 c = vec3(1,1./16.,1./16.) * exp(4.*d - 1.);
//    col = c;

    fragColor = spikeColor(d, time, worldPosition, normal);
    //fragColor = vec4(vec3(position.x), 1.0);
}
