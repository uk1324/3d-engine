#version 430 core

in vec2 position; 
in vec2 worldPosition; 

in float time; 
out vec4 fragColor;

/*generated end*/

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

void main() {
//    float len = 1.0;
//    float a = position.x / len;
//    //outColor = vec4(texturePos, 0.0, 1.0 - texturePos.y);
//    //outColor = vec4(vec3(texturePos.y), texturePosition.y);
//    //texturePos.y /= texturePos.x;
//    //texturePos.y -= octave01(vec2(texturePos.x / len * 10.0, 0.0), 4) * 0.2;
//    vec2 p = position;
//    p.x /= len;
//    p.x += time;
//    float d = octave01(p, 4);
//    p = position;
//    p.x /= len;
//    d *= 1.0 - p.y;
//    d *= a;
//    //d = 1.0 - a;
//    //d *= 1.0 - a;
//    d *= 1.0 - smoothstep(0.2, 0.0, 1.0 - p.x);
//    vec3 c = vec3(1,1./4.,1./16.) * exp(4.*d - 1.);
//    fragColor = vec4(vec3(d), d);
//    fragColor = vec4(c, d);



    float len = 1.0;
    float a = position.x / len;
    //outColor = vec4(texturePos, 0.0, 1.0 - texturePos.y);
    //outColor = vec4(vec3(texturePos.y), texturePosition.y);
    //texturePos.y /= texturePos.x;
    //texturePos.y -= octave01(vec2(texturePos.x / len * 10.0, 0.0), 4) * 0.2;
//    vec2 p = position;
//    p.x /= len;
//    p.x += time;
//    float d = octave01(worldPosition / 209.0 + vec2(time, 0), 4);
//    p = position;
//    p.x /= len;
//    //d *= 1.0 - p.y;
//    d *= a;
//    //d = 1.0 - a;
//    //d *= 1.0 - a;
//    float d = octave01(worldPosition / 209.0 + vec2(time, 0), 4);
//    d *= 1.0 - smoothstep(0.2, 0.0, 1.0 - position.x);
//    vec3 c = vec3(1,1./4.,1./16.) * exp(4.*d - 1.);
//    fragColor = vec4(vec3(d), d);
//    fragColor = vec4(c, d);

    

    vec3 col;
//    {
//            float d = position.x;
//    d = 1.0 / (d * d);
//    d = octave01(worldPosition / 200.00 + vec2(time, 0), 4);
//    d *= position.x;
//    vec3 c = vec3(1,1./4.,1./16.) * exp(4.*d - 1.);
//
//    col = vec3(d);
//    col = vec3(c);
//    }
    float d = 1.0 - position.x;
   // d += octave01(worldPosition / 4000.00 + vec2(time, 0), 1);
    d += octave01(vec2(time, worldPosition.y / 200.0), 1) * 0.4 - 0.05;
    d *= 2.5 * 2.5;
    d = 1.0 / (d  * d);
    col = vec3(d);
//
    vec3 c = vec3(1,1./16.,1./16.) * exp(4.*d - 1.);
    col = c;

    fragColor = vec4(col, d);
    //fragColor = vec4(vec3(1.0), 1.0);
}
