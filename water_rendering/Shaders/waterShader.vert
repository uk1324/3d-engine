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

#define DRAG_MULT 0.28 // changes how much waves pull on the water
#define WATER_DEPTH 1.0 // how deep is the water
#define ITERATIONS_RAYMARCH 12 // waves iterations of raymarching

// Calculates wave value and its derivative, 
// for the wave direction, position in space, wave frequency and time
vec2 wavedx(vec2 position, vec2 direction, float frequency, float timeshift) {
  float x = dot(direction, position) * frequency + timeshift;
  float wave = exp(sin(x) - 1.0);
  float dx = wave * cos(x);
  return vec2(wave, -dx);
}

// Calculates waves by summing octaves of various waves with various parameters
float getwaves(vec2 position, int iterations) {
  float iter = 0.0; // this will help generating well distributed wave directions
  float frequency = 1.0; // frequency of the wave, this will change every iteration
  //float timeMultiplier = 2.0; // time multiplier for the wave, this will change every iteration
  //float timeMultiplier = 0.1; // time multiplier for the wave, this will change every iteration
  float timeMultiplier = 0.15; // time multiplier for the wave, this will change every iteration
  float weight = 1.0;// weight in final sum for the wave, this will change every iteration
  float sumOfValues = 0.0; // will store final sum of values
  float sumOfWeights = 0.0; // will store final sum of weights
  for(int i=0; i < iterations; i++) {
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
  return sumOfValues / sumOfWeights * 1.5;
}

// Calculate normal at point by calculating the height at the pos and 2 additional points very close to pos
vec3 normal(vec2 pos, float e, float depth, int iterations) {
  vec2 ex = vec2(e, 0);
  float H = getwaves(pos.xy, iterations) * depth;
  vec3 a = vec3(pos.x, H, pos.y);
  return normalize(
    cross(
      a - vec3(pos.x - e, getwaves(pos.xy - ex.xy, iterations) * depth, pos.y), 
      a - vec3(pos.x, getwaves(pos.xy + ex.yx, iterations) * depth, pos.y + e)
    )
  );
}

// Helper function generating a rotation matrix around the axis by the angle
mat3 createRotationMatrixAxisAngle(vec3 axis, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
  return mat3(
    oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 
    oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 
    oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
  );
}


// Ray-Plane intersection checker
float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal) { 
  return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0); 
}

// Main
void main() {
    vec2 position = vertexPosition + instanceOffset;
    position /= 10.0;

    float height = getwaves(position, ITERATIONS_RAYMARCH) * WATER_DEPTH - WATER_DEPTH;
    height *= 10.0;
    float dist = length(position);

    iterations = ITERATIONS_NORMAL;
    iterations = int(mix(maxIterations, 1, smoothstep(maxQualityDistance, minQualityDistance, length(vertexPosition + instanceOffset))));
    vec3 N = normal(position, 0.01, WATER_DEPTH, iterations);

    // smooth the normal with distance to avoid disturbing high frequency noise
    //N = mix(N, vec3(0.0, 1.0, 0.0), 0.8 * min(1.0, sqrt(dist*0.01) * 1.1));

    unnormalizedNormal = N;

    position = vertexPosition + instanceOffset;
    fragmentWorldPosition = vec3(position.x, height, position.y);
    gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
}