struct Wave {
    float height;
    float derivative;
};


#define DRAG_MULT 0.28 // changes how much waves pull on the water
#define ITERATIONS_RAYMARCH 12 // waves iterations of raymarching
#define ITERATIONS_NORMAL 40 // waves iterations when calculating normals

Wave sampleWave(vec2 position, vec2 direction, float frequency, float translation) {
    float x = dot(direction, position) * frequency + translation;
    Wave r;
    r.height = exp(sin(x) - 1.0);
    r.derivative = -(r.height * cos(x));
    return r;
}

vec2 wavedx(vec2 position, vec2 direction, float frequency, float timeshift) {
    float x = dot(direction, position) * frequency + timeshift;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return vec2(wave, -dx);
}

float sampleWaves(vec2 position, int iterations) {
    float iter = 0.0; // this will help generating well distributed wave directions
    float frequency = 1.0; // frequency of the wave, this will change every iteration
    float timeMultiplier = 0.15; // time multiplier for the wave, this will change every iteration
    float weight = 1.0;// weight in final sum for the wave, this will change every iteration
    float sumOfValues = 0.0; // will store final sum of values
    float sumOfWeights = 0.0; // will store final sum of weights
    for (int i = 0; i < iterations; i++) {
        // generate some wave direction that looks kind of random
        vec2 p = vec2(sin(iter), cos(iter));
        // calculate wave data
        vec2 res = wavedx(position, p, frequency, time * timeMultiplier);

        // shift position around according to wave drag and derivative of the wave
        position += p * res.y * weight * DRAG_MULT;

        // add the results to sums
        sumOfValues += res.x * weight;
        sumOfWeights += weight;

        // modify next octave parameters
        weight *= 0.82;
        frequency *= 1.18;
        timeMultiplier *= 1.07;

        // add some kind of random value to make next wave look random too
        iter += 1232.399963;
    }
    // calculate and return
    return sumOfValues / sumOfWeights;
    //float frequency = 1.0;
    //float speed = 0.15;
    //float weight = 1.0;

    //float sumOfValues = 0.0;
    //// For normalizing.
    //float sumOfWeights = 0.0; // Could compute this using sum of geometric series.

    //float noise = 0.0;
    //for (int i = 0; i < iterations; i++) {
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
    //return sumOfValues / sumOfWeights;
}

vec2 h = vec2(0.001, 0.0);
vec2 derivatives(vec2 pos, int iterations) {
    float a = sampleWaves(pos, iterations);
    return vec2(
        (sampleWaves(pos + h.xy, iterations) - a) / h.x,
        (sampleWaves(pos + h.yx, iterations) - a) / h.x
    );
}

vec4 jacobian(vec2 pos, int iterations) {
    float a = sampleWaves(pos, iterations);
    return vec4(
        (derivatives(pos + h.xy, iterations) - a) / h.x,
        (derivatives(pos + h.yx, iterations) - a) / h.x
    );
}

vec3 sampleWaveNormal(vec2 pos, float depth, int iterations) {
    float e = 0.001;
    vec2 ex = vec2(e, 0);
    float H = sampleWaves(pos.xy, iterations) * depth;
    vec3 a = vec3(pos.x, H, pos.y);
    return normalize(
    cross(
    a - vec3(pos.x - e, sampleWaves(pos.xy - ex.xy, iterations) * depth, pos.y),
    a - vec3(pos.x, sampleWaves(pos.xy + ex.yx, iterations) * depth, pos.y + e)
    )
    );
}
//
//vec3 sampleWaveNormal(vec2 pos, float depth, int iterations) {
//    vec2 h = vec2(0.01, 0);
//    float height = sampleWaves(pos.xy, iterations) * depth;
//    vec3 a = vec3(pos.x, height, pos.y);
//    return normalize(cross(
//        a - vec3(pos.x - h.x, sampleWaves(pos.xy - h.xy, iterations) * depth, pos.y),
//        a - vec3(pos.x, sampleWaves(pos.xy + h.yx, iterations) * depth, pos.y + h.x)
//    ));
//}