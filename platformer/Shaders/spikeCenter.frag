#version 430 core

uniform float time; 

in vec2 position; 
in vec2 worldPosition; 

in vec2 normal; 
out vec4 fragColor;

/*generated end*/

#include "spikeShared.glsl"

void main() {
    vec3 col;
    float d = position.x;
    fragColor = spikeColor(d, time, worldPosition, normal);
}
