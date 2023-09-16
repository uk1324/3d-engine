#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in vec2 instanceOffset; 

uniform float time; 
uniform float maxQualityDistance; 
uniform float minQualityDistance; 
uniform int maxIterations; 

out vec3 unnormalizedNormal; 
out vec3 fragmentWorldPosition; 
out flat int iterations; 

/*generated end*/

#include "waves.glsl"

void main() {
    vec2 position = vertexPosition + instanceOffset;
    float scale = 10.0;
    vec2 waveCooridnate = position / scale;

    float WATER_DEPTH = 1.0;

    float height = sampleWaves(waveCooridnate, 12) * WATER_DEPTH - WATER_DEPTH;
    height *= scale;

    iterations = int(mix(maxIterations, 1, smoothstep(maxQualityDistance, minQualityDistance, length(vertexPosition + instanceOffset))));
    //unnormalizedNormal = sampleWaveNormal(waveCooridnate, WATER_DEPTH, iterations);

    fragmentWorldPosition = vec3(position.x, height, position.y);
    gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
}