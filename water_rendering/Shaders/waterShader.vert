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

struct Wave {
    float height;
    float derivative;
};

Wave wavedx(vec2 position, vec2 direction, float frequency, float translation) {
    float x = dot(direction, position) * frequency + translation;
    Wave r;
    r.height = exp(sin(x) - 1.0);
    r.derivative = -(r.height * cos(x));
    return r;
}

float sampleWaves(vec2 position, int iterations) {
    float frequency = 1.0; 
    float speed = 0.15;
    float weight = 1.0;

    float sumOfValues = 0.0;
    // For normalizing.
    float sumOfWeights = 0.0; // Could compute this using sum of geometric series.

    float noise = 0.0;
    for (int i = 0; i < iterations; i++) {
        vec2 direction = vec2(sin(noise), cos(noise));

        Wave wave = wavedx(position, direction, frequency, time * speed);

        float waveDrag = 0.28;
        position += direction * wave.derivative * weight * waveDrag;

        sumOfValues += wave.height * weight;
        sumOfWeights += weight;

        weight *= 0.82;
        frequency *= 1.18;
        speed *= 1.07;

        noise += 1232.399963;
    }

    return sumOfValues / sumOfWeights * 1.5;
}

vec3 normal(vec2 pos, float depth, int iterations) {
    vec2 h = vec2(0.01, 0);
    float height = sampleWaves(pos.xy, iterations) * depth;
    vec3 a = vec3(pos.x, height, pos.y);
    return normalize(cross(
        a - vec3(pos.x - h.x, sampleWaves(pos.xy - h.xy, iterations) * depth, pos.y), 
        a - vec3(pos.x, sampleWaves(pos.xy + h.yx, iterations) * depth, pos.y + h.x)
    ));
}

void main() {
    vec2 position = vertexPosition + instanceOffset;
    float scale = 10.0;
    vec2 waveCooridnate = position / scale;

    float WATER_DEPTH = 1.0;

    float height = sampleWaves(waveCooridnate, 12) * WATER_DEPTH - WATER_DEPTH;
    height *= scale;

    iterations = int(mix(maxIterations, 1, smoothstep(maxQualityDistance, minQualityDistance, length(vertexPosition + instanceOffset))));
    unnormalizedNormal = normal(waveCooridnate, WATER_DEPTH, iterations);

    fragmentWorldPosition = vec3(position.x, height, position.y);
    gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
}