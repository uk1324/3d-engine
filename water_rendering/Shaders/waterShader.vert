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

//float hash21(vec2 co){
//    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
//}
//
//float hash11(float p) {
//	vec3 p3  = fract(vec3(p) * 0.1031);
//    p3 += dot(p3, p3.yzx + 19.19);
//    return fract((p3.x + p3.y) * p3.z);
//}
//
//float getHeight(vec2 p) {
//	float t = time / 2.0;
//	p += vec2(t, 0.0);
//	p /= 10.0;
//
//	float h = 0.0;
//
//	float scale = 0.75;
//	{
//		float amplitude = 1.0;
//		float frequency = 1.0;
//		int iterations = 6;
//		for (int i = 0; i < iterations; i++) {
//			float angle = hash11(i);
//			float d = dot(vec2(cos(angle), sin(angle)), p);
//			h += amplitude * exp(sin(frequency * d + t / 3.0));
//			amplitude *= scale;
//			frequency /= scale;
//		}
//	}
//
//	return h;
//
//}
//
//void main() {
//	vec2 position = vertexPosition + instanceOffset;
//	vec3 v0 = vec3(0.0, getHeight(position), 0.0);
//	float stp = 1.0;
//	vec3 v1 = vec3(stp, getHeight(vec2(position.x + stp, position.y)), 0.0);
//	vec3 v2 = vec3(0.0, getHeight(vec2(position.x, position.y + stp)), stp);
//	// After interpolation it has to be normalized so it isn't normalized here.
//
//	unnormalizedNormal = cross(v1 - v0, v2 - v0);
//	fragmentWorldPosition = vec3(position.x, v0.y, position.y);
//	gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
//
//	{
//		vec2 p = position;
//		vec2 h = vec2(0.1, 0.0);
//		unnormalizedNormal = normalize(vec3(
//			getHeight(p-h.xy) - getHeight(p+h.xy),
//            2.0 * h.x,
//            getHeight(p-h.yx) - getHeight(p+h.yx)
//		));
//	}
//}

//#define DRAG_MULT 0.28 // changes how much waves pull on the water
//#define WATER_DEPTH 1.0 // how deep is the water
//#define CAMERA_HEIGHT 1.5 // how high the camera should be
////#define ITERATIONS_RAYMARCH 12 // waves iterations of raymarching
////#define ITERATIONS_NORMAL 24 // waves iterations when calculating normals
////#define ITERATIONS_RAYMARCH 12 // waves iterations of raymarching
////#define ITERATIONS_NORMAL 16 // waves iterations when calculating normals
//#define ITERATIONS_RAYMARCH 16 // waves iterations of raymarching
//#define ITERATIONS_NORMAL 16 // waves iterations when calculating normals
//
//// Calculates wave value and its derivative, 
//// for the wave direction, position in space, wave frequency and time
//vec2 wavedx(vec2 position, vec2 direction, float frequency, float timeshift) {
//  float x = dot(direction, position) * frequency + timeshift;
//  float wave = exp(sin(x) - 1.0);
//  float dx = wave * cos(x);
//  return vec2(wave, -dx);
//}
//
//// Calculates waves by summing octaves of various waves with various parameters
//float getwaves(vec2 position, int iterations) {
//  float iter = 0.0; // this will help generating well distributed wave directions
//  float frequency = 1.0; // frequency of the wave, this will change every iteration
//  //float timeMultiplier = 2.0; // time multiplier for the wave, this will change every iteration
//  float timeMultiplier = 0.1; // time multiplier for the wave, this will change every iteration
//  float weight = 1.0;// weight in final sum for the wave, this will change every iteration
//  float sumOfValues = 0.0; // will store final sum of values
//  float sumOfWeights = 0.0; // will store final sum of weights
//  for(int i=0; i < iterations; i++) {
//    // generate some wave direction that looks kind of random
//    vec2 p = vec2(sin(iter), cos(iter));
//    // calculate wave data
//    vec2 res = wavedx(position, p, frequency, time * timeMultiplier);
//
//    // shift position around according to wave drag and derivative of the wave
//    position += p * res.y * weight * DRAG_MULT;
//
//    // add the results to sums
//    sumOfValues += res.x * weight;
//    sumOfWeights += weight;
//
//    // modify next octave parameters
//    weight *= 0.82;
//    frequency *= 1.18;
//    timeMultiplier *= 1.07;
//
//    // add some kind of random value to make next wave look random too
//    iter += 1232.399963;
//  }
//  // calculate and return
//  //return 0.0;
//  return sumOfValues / sumOfWeights * 20.0;
//}
//
//// Calculate normal at point by calculating the height at the pos and 2 additional points very close to pos
//vec3 normal(vec2 pos, float e, float depth) {
//  vec2 ex = vec2(e, 0);
//    //int iterations = int(smoothstep(50.0, 500.0, length(pos)) * ITERATIONS_NORMAL);
//    int iterations = int(exp(-smoothstep(50.0, 500.0, length(pos))) * ITERATIONS_NORMAL);
//    //int iterations = ITERATIONS_NORMAL;
//  float H = getwaves(pos.xy, iterations) * depth;
//  vec3 a = vec3(pos.x, H, pos.y);
//  return normalize(
//    cross(
//      a - vec3(pos.x - e, getwaves(pos.xy - ex.xy, iterations) * depth, pos.y), 
//      a - vec3(pos.x, getwaves(pos.xy + ex.yx, iterations) * depth, pos.y + e)
//    )
//  );
//}
//
//// Helper function generating a rotation matrix around the axis by the angle
//mat3 createRotationMatrixAxisAngle(vec3 axis, float angle) {
//  float s = sin(angle);
//  float c = cos(angle);
//  float oc = 1.0 - c;
//  return mat3(
//    oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 
//    oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 
//    oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
//  );
//}
//
//
//// Ray-Plane intersection checker
//float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal) { 
//  return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0); 
//}
//
//// Great tonemapping function from my other shader: https://www.shadertoy.com/view/XsGfWV
//vec3 aces_tonemap(vec3 color) {  
//  mat3 m1 = mat3(
//    0.59719, 0.07600, 0.02840,
//    0.35458, 0.90834, 0.13383,
//    0.04823, 0.01566, 0.83777
//  );
//  mat3 m2 = mat3(
//    1.60475, -0.10208, -0.00327,
//    -0.53108,  1.10813, -0.07276,
//    -0.07367, -0.00605,  1.07602
//  );
//  vec3 v = m1 * color;  
//  vec3 a = v * (v + 0.0245786) - 0.000090537;
//  vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
//  return pow(clamp(m2 * (a / b), 0.0, 1.0), vec3(1.0 / 2.2));  
//}
//
//// Main
//void main() {
//    
//  // get the ray
////    vec3 ray = getRay(fragCoord);
////    if(ray.y >= 0.0) {
////    // if ray.y is positive, render the sky
////    vec3 C = getAtmosphere(ray) + getSun(ray);
////    fragColor = vec4(aces_tonemap(C * 2.0),1.0);   
////    return;
////    }
//
//    // now ray.y must be negative, water must be hit
//    // define water planes
////    vec3 waterPlaneHigh = vec3(0.0, 0.0, 0.0);
////    vec3 waterPlaneLow = vec3(0.0, -WATER_DEPTH, 0.0);
//
//    // define ray origin, moving around
////    vec3 origin = vec3(iTime, CAMERA_HEIGHT, iTime);
////
////    // calculate intersections and reconstruct positions
////    float highPlaneHit = intersectPlane(origin, ray, waterPlaneHigh, vec3(0.0, 1.0, 0.0));
////    float lowPlaneHit = intersectPlane(origin, ray, waterPlaneLow, vec3(0.0, 1.0, 0.0));
////    vec3 highHitPos = origin + ray * highPlaneHit;
////    vec3 lowHitPos = origin + ray * lowPlaneHit;
////
//    vec2 position = vertexPosition + instanceOffset;
//    position /= 20.0;
//    // raymatch water and reconstruct the hit pos
//    float height = getwaves(position, ITERATIONS_RAYMARCH) * WATER_DEPTH - WATER_DEPTH;
//    //height = 0.0;
//    //height *= 10.0;
//    float dist = length(position);
//
//    vec3 N = normal(position, 0.01, WATER_DEPTH);
//   // N = vec3(0, 1, 0);
//
//    // smooth the normal with distance to avoid disturbing high frequency noise
//    N = mix(N, vec3(0.0, 1.0, 0.0), 0.8 * min(1.0, sqrt(dist*0.01) * 1.1));
//
//    unnormalizedNormal = N;
//
//    position = vertexPosition + instanceOffset;
//    fragmentWorldPosition = vec3(position.x, height, position.y);
//    gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
//
//
////    vec2 position = vertexPosition + instanceOffset;
////    float height = 0.0;
////    fragmentWorldPosition = vec3(position.x, height, position.y);
////    unnormalizedNormal = vec3(0.0, 1.0, 0.0);
////    gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
//
//}

#define DRAG_MULT 0.28 // changes how much waves pull on the water
#define WATER_DEPTH 1.0 // how deep is the water
#define CAMERA_HEIGHT 1.5 // how high the camera should be
#define ITERATIONS_RAYMARCH 12 // waves iterations of raymarching
//#define ITERATIONS_NORMAL 40 // waves iterations when calculating normals
//#define ITERATIONS_NORMAL 23 // waves iterations when calculating normals
#define ITERATIONS_NORMAL 23 // waves iterations when calculating normals

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
  // get the ray
//    vec3 ray = getRay(fragCoord);
//    if(ray.y >= 0.0) {
//    // if ray.y is positive, render the sky
//    vec3 C = getAtmosphere(ray) + getSun(ray);
//    fragColor = vec4(aces_tonemap(C * 2.0),1.0);   
//    return;
//    }

    // now ray.y must be negative, water must be hit
    // define water planes
//    vec3 waterPlaneHigh = vec3(0.0, 0.0, 0.0);
//    vec3 waterPlaneLow = vec3(0.0, -WATER_DEPTH, 0.0);

    // define ray origin, moving around
//    vec3 origin = vec3(iTime, CAMERA_HEIGHT, iTime);
//
//    // calculate intersections and reconstruct positions
//    float highPlaneHit = intersectPlane(origin, ray, waterPlaneHigh, vec3(0.0, 1.0, 0.0));
//    float lowPlaneHit = intersectPlane(origin, ray, waterPlaneLow, vec3(0.0, 1.0, 0.0));
//    vec3 highHitPos = origin + ray * highPlaneHit;
//    vec3 lowHitPos = origin + ray * lowPlaneHit;

    // raymatch water and reconstruct the hit pos
    float height = getwaves(position, ITERATIONS_RAYMARCH) * WATER_DEPTH - WATER_DEPTH;
    height *= 10.0;
    float dist = length(position);

    iterations = ITERATIONS_NORMAL;
    iterations = int(mix(maxIterations, 1, smoothstep(maxQualityDistance, minQualityDistance, length(vertexPosition + instanceOffset))));
    vec3 N = normal(position, 0.01, WATER_DEPTH, iterations);

    // smooth the normal with distance to avoid disturbing high frequency noise
    N = mix(N, vec3(0.0, 1.0, 0.0), 0.8 * min(1.0, sqrt(dist*0.01) * 1.1));

    unnormalizedNormal = N;

    position = vertexPosition + instanceOffset;
    fragmentWorldPosition = vec3(position.x, height, position.y);
    gl_Position = instanceTransform * vec4(fragmentWorldPosition, 1.0);
}