#version 430 core

in vec2 position; 

in float time; 
out vec4 fragColor;

/*generated end*/

float boxSDF(in vec2 p, in vec2 box) {
	return length(max(abs(p) - box, vec2(0.0)));
}

float f(float x) {
	if (x < 2.0) {
		return 1.0 / x;
	}
	if (x < 4.0) {
		return -0.25 * x + 1.0;
	}
	return 0.0;
}


float f2(float x) {
	if (x < 4.0) {
		return 1.0 / x;
	}
	if (x < 8.0) {
		return -0.125 * x + 0.5;
	}
	return 0.0;
}

float f3(float x) {
	float p = 5.00;
	if (x < p) {
		return 1.0 / x;
	}
	return p / (x * x);
}

void main() {
	float s = 1.0;
	s *= 0.25 / 2;

	float d = boxSDF(position, vec2(s * (2.0 / 3.0), s));
	float d1 = d;
	d *= 64.0;
	//d = f2(d);
	//d = 1.0 / d;
	d = f3(d);
	//d += sin(time) * 0.0321;
	//d = 1 / d;
	//d = step(d, 0.0001);
//	d = 1.0 / (d * d);
	//d = 1.0;
	fragColor = vec4(vec3(d), d);
}
