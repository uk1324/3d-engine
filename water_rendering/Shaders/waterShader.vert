#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 
layout(location = 5) in vec2 instanceOffset; 

uniform float time; 
uniform float maxQualityDistance; 
uniform float minQualityDistance; 
uniform int maxIterations; 
uniform bool useAnalyticalDerivatives; 

out vec3 unnormalizedNormal; 
out vec3 fragmentWorldPosition; 
out flat int iterations; 

/*generated end*/

#include "waves.glsl"
#include "waterDepth.glsl"

void main() {
    vec2 position = vertexPosition + instanceOffset;
    float scale = 10.0;
    vec2 waveCooridnate = position / scale;

//    float height = sampleWaves(waveCooridnate, 12);

    float height = sampleWaves(waveCooridnate, 12) * WATER_DEPTH - WATER_DEPTH;
    height *= scale;

    iterations = int(mix(maxIterations, 1, smoothstep(maxQualityDistance, minQualityDistance, length(vertexPosition + instanceOffset))));
    //unnormalizedNormal = sampleWaveNormal(waveCooridnate, WATER_DEPTH, iterations);

    //vec2 ds = normalize(derivatives(waveCooridnate, 12));
    vec2 ds = derivatives(waveCooridnate, 12) * 12.0;

    position = vertexPosition + instanceOffset;
    fragmentWorldPosition = vec3(position.x, height, position.y) + vec3(ds.x, 0.0, ds.y);
    gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
}