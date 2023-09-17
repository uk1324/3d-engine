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

    //frequency = 0.3;
    //speed = 0.15;
    //weight = 1.0;
    //for (int i = 0; i < 3; i++) {
    //    vec2 direction = vec2(sin(noise), cos(noise));

    //    Wave wave = sampleWave(position, direction, frequency, time * speed);

    //    float waveDrag = 0.28;
    //    position += direction * wave.derivative * weight * waveDrag;

    //    sumOfValues += wave.height * weight;
    //    sumOfWeights += weight;

    //    weight *= 0.82;
    //    frequency *= 1.18;
    //    speed *= 1.07;

    //    noise += 1232.399963;
    //}

    //return sumOfValues / sumOfWeights * 1.5;
    //return sumOfValues / sumOfWeights * 1.5 - 1.0;
    //return sumOfValues / sumOfWeights;
    return sumOfValues;
    //return sin(position.x);
}

vec3 sampleWaveNormal(vec2 pos, int iterations) {
    vec2 position = pos;
    float frequency = 1.0;
    float speed = 0.15;
    float weight = 1.0;

    float sumOfValues = 0.0;
    // For normalizing.
    float sumOfWeights = 0.0; // Could compute this using sum of geometric series.

    float noise = 0.0;

    float dhdx = 0.0;
    float dhdy = 0.0;

    for (int i = 0; i < iterations; i++) {
        vec2 direction = vec2(sin(noise), cos(noise));

        Wave wave = sampleWave(position, direction, frequency, time * speed);
        // The derivative at the perpendicular direction to `direction` is equal to 0.
        dhdx += direction.x * -wave.derivative;
        dhdy += direction.y * -wave.derivative;

        float waveDrag = 0.28;
        position += direction * wave.derivative * weight * waveDrag;

        //sumOfValues += wave.height * weight;
        //sumOfWeights += weight;

        weight *= 0.82;
        frequency *= 1.18;
        speed *= 1.07;

        noise += 1232.399963;
    }
    //sumOfValues /= sumOfWeights * 1.5;
    //dhdx *= sumOfWeights * 1.5;
    //dhdy *= sumOfWeights * 1.5;
    //dhdy = abs(dhdy);
    //dhdx = abs(dhdx);

    vec2 h = vec2(0.00001, 0);
    float height = sampleWaves(pos.xy, iterations);
    vec3 a = vec3(pos.x, height, pos.y);

    if (useAnalyticalDerivatives) {
        //return normalize(cross(vec3(1.0, dhdx, 0.0), vec3(0.0, dhdy, 1.0)));
        /*return normalize(cross(
            vec3(h.x, height - sampleWaves(pos.xy - h.xy, iterations), 0.0),
            vec3(0.0, height - sampleWaves(pos.xy + h.yx, iterations), h.x)
        ));*/
        return vec3(dhdx, dhdy, 0.0);
    }
    

    return vec3(
        (sampleWaves(pos.xy + h.xy, iterations) - height) / h.x,
        (sampleWaves(pos.xy + h.yx, iterations) - height) / h.x,
        0.0f
    );
    //return normalize(cross(
    //    a - vec3(pos.x - h.x, sampleWaves(pos.xy - h.xy, iterations), pos.y),
    //    a - vec3(pos.x, sampleWaves(pos.xy + h.yx, iterations), pos.y + h.x)
    //));
    /*return normalize(cross(
        a - vec3(pos.x - h.x, sampleWaves(pos.xy - h.xy, iterations), pos.y),
        a - vec3(pos.x, sampleWaves(pos.xy + h.yx, iterations), pos.y + h.x)
    ));*/


    //vec2 h = vec2(0.01, 0);
    //float height = sampleWaves(pos.xy, iterations) * depth;
    //vec3 a = vec3(pos.x, height, pos.y);
    //return normalize(cross(
    //    a - vec3(pos.x - h.x, sampleWaves(pos.xy - h.xy, iterations) * depth, pos.y),
    //    a - vec3(pos.x, sampleWaves(pos.xy + h.yx, iterations) * depth, pos.y + h.x)
    //));
}