struct Wave {
    float height;
    float derivative;
};

Wave sampleWave(vec2 position, vec2 direction, float frequency, float translation) {
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

        Wave wave = sampleWave(position, direction, frequency, time * speed);

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

vec3 sampleWaveNormal(vec2 pos, float depth, int iterations) {
    vec2 h = vec2(0.01, 0);
    float height = sampleWaves(pos.xy, iterations) * depth;
    vec3 a = vec3(pos.x, height, pos.y);
    return normalize(cross(
        a - vec3(pos.x - h.x, sampleWaves(pos.xy - h.xy, iterations) * depth, pos.y),
        a - vec3(pos.x, sampleWaves(pos.xy + h.yx, iterations) * depth, pos.y + h.x)
    ));
}