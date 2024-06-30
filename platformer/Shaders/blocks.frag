#version 430 core

uniform float time; 
uniform float scale; 

in vec2 worldPosition; 
out vec4 fragColor;

/*generated end*/

float hash(float n)
{
    return fract(sin(n)*43758.5453);
}
float noise(vec2 p)
{
    return hash(p.x + p.y*57.0);
}

float maze(vec2 p)
{
   vec2 p2 = (p - .5) * mat2(1, 1, 1, -1);
   vec2 c = floor(p2), f = fract(p);
   return step(abs(mix(f.x - .5, f.y - .5, step(.5, noise(c)))), .05);
}


float maze2(vec2 p)
{
   vec2 p2 = (p - .5) * mat2(1, 1, 1, -1);
   vec2 c = floor(p2), f = fract(p);
   return abs(mix(f.x - .5, f.y - .5, step(.5, noise(c))));
}


/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

/* skew constants for 3d simplex functions */
const float F3 =  0.3333333;
const float G3 =  0.1666667;

/* 3d simplex noise */
float simplex3d(vec3 p) {
	 /* 1. find current tetrahedron T and it's four vertices */
	 /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	 /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/
	 
	 /* calculate s and x */
	 vec3 s = floor(p + dot(p, vec3(F3)));
	 vec3 x = p - s + dot(s, vec3(G3));
	 
	 /* calculate i1 and i2 */
	 vec3 e = step(vec3(0.0), x - x.yzx);
	 vec3 i1 = e*(1.0 - e.zxy);
	 vec3 i2 = 1.0 - e.zxy*(1.0 - e);
	 	
	 /* x1, x2, x3 */
	 vec3 x1 = x - i1 + G3;
	 vec3 x2 = x - i2 + 2.0*G3;
	 vec3 x3 = x - 1.0 + 3.0*G3;
	 
	 /* 2. find four surflets and store them in d */
	 vec4 w, d;
	 
	 /* calculate surflet weights */
	 w.x = dot(x, x);
	 w.y = dot(x1, x1);
	 w.z = dot(x2, x2);
	 w.w = dot(x3, x3);
	 
	 /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	 w = max(0.6 - w, 0.0);
	 
	 /* calculate surflet components */
	 d.x = dot(random3(s), x);
	 d.y = dot(random3(s + i1), x1);
	 d.z = dot(random3(s + i2), x2);
	 d.w = dot(random3(s + 1.0), x3);
	 
	 /* multiply d by w^4 */
	 w *= w;
	 w *= w;
	 d *= w;
	 
	 /* 3. return the sum of the four surflets */
	 return dot(d, vec4(52.0));
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

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

float j(vec2 p) {
	//return maze2(p + 0.5 * vec2( octave01(p, 1),  octave01(p + vec2(123, 34532), 1)));// * octave01(p, 1);
	float t = time / 10;
	return maze2(p + octave01(p, 2) * vec2(simplex3d(vec3(p, t)),  simplex3d(vec3(p + vec2(123, 34532), t))));// * octave01(p, 1);
    //return simplex3d(vec3(p, 0.0));
}

vec3 getNormal2(vec2 p )
{
    const float eps = 0.01; // or some other value
    const vec2 h = vec2(eps,0);
    return normalize( vec3( j(p-h.xy) - j(p+h.xy),
                            2.0*h.x,
                            j(p-h.yx) - j(p+h.yx) ) );
}

void main() {
//    float noise = simplex3d(vec3(worldPosition / 200.0, time / 5.0));
//    float c = dot(getNormal2(worldPosition / 200.0), normalize(vec3(1.0, 1.0, 0.0)));
//    c = pow(c, 20.0);
//    vec3 ca = hsv2rgb(vec3(c, 1.0, 1.0));
//	vec3 col = vec3(maze(worldPosition / 20.0) * (clamp(0.1 + noise * 0.5, 0, 1)));

	float noise = simplex3d(vec3(worldPosition / 200.0, time / 5.0));
    float c = dot(getNormal2(worldPosition / 200.0), normalize(vec3(1.0, 1.0, 0.0)));
    c = pow(c, 20.0);
    vec3 ca = hsv2rgb(vec3(c, 1.0, 1.0));
	float m = maze2(worldPosition / 20.0) * 0.5 + octave01(worldPosition / 200.0, 4) * 0.5;
	vec3 col = vec3(dot(getNormal2(worldPosition / 60.0), normalize(vec3(1, 1, 0))));
	float n = simplex3d(vec3(worldPosition, time));
	n += 1.0;
	n /= 2.0;
	//col *= 0.1 * simplex3d(vec3(worldPosition / 100.0, time / 20.0));
	if (scale == 1.0) {
		col *= 0.1 * clamp(simplex3d(vec3(worldPosition / 100.0, time / 20.0)), 0, 1);
	} else {
		col *= 0.5 * clamp(simplex3d(vec3(worldPosition / 100.0, time / 20.0)), 0, 1);
	}
	//col *= hsv2rgb(vec3(m * 4.0, 1.0, 1.0));
//	vec2 pos = worldPosition / 600;
//	float t = time / 100;
//    for (int i = 0; i < 3; i++) {
//        mat2 m = mat2(vec2(1.0, 0.0), vec2(1.0, 5.0));
//        pos += vec2(simplex3d(vec3(pos, t)), simplex3d(vec3(pos + vec2(123.123, 2.34), t)));
//        pos *= m;
//    }
//    col = vec3(simplex3d(vec3(pos, 2)));

	fragColor = vec4(col * scale, 1.0);
}
