vec2 hash(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float perlin01(vec2 p) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

    vec2  i = floor(p + (p.x + p.y) * K1);
    vec2  a = p - i + (i.x + i.y) * K2;
    float m = step(a.y, a.x);
    vec2  o = vec2(m, 1.0 - m);
    vec2  b = a - o + K2;
    vec2  c = a - 1.0 + 2.0 * K2;
    vec3  h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
    vec3  n = h * h * h * h * vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
    return (dot(n, vec3(70.0)) + 1.0) * 0.5;
}

float octave01(vec2 p, int octaves) {
    float amplitude = .5;
    float frequency = 0.;
    float value = 0.0;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * perlin01(p);
        p *= 2.;
        amplitude *= .5;
    }
    return value;
}

vec4 spikeColor(float d, float time, vec2 worldPosition, vec2 normal) {
    //return vec4(vec3(d), 1.0);
    //return vec4(vec3(d), d);
    //return vec4(normal, 0.0, 1.0);
    //return vec4((normal + 1.0) / 2.0, 0.0, 1.0);
    worldPosition /= 200.0;
    vec2 pos = (vec2(perlin01(worldPosition), perlin01(worldPosition + vec2(123, 532))) - 0.5) * 2.0;
    //return vec4(pos, 0.0, 1.0);

    //return vec4(vec3(d), 1.0);
    // Istead of doing the dot product of the random pos could just generate a random pos along, but I think it looks better with the dot product.
    float posAlong = dot(normal, pos);
    posAlong += 1.0;
    posAlong /= 2.0;

    //return vec4(vec3(posAlong), 1.0);
    d = 1.0 - d;
    d += octave01(vec2(time / 2.0, posAlong), 3) * 0.3 - 0.08;
    d *= 2.5 * 2.5;
    d = 1.0 / (d  * d);

    return vec4(vec3(1, 1.0 / 16.0, 1.0 / 16.0) * exp(4.0 * d - 1.0), d);
}