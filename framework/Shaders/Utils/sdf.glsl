float circleSdf(vec2 p, vec2 center, float r) {
	return distance(center, p) - r;
}

float intersectSdf(float d0, float d1) {
	return max(d0, d1);
}

float subtractSdf(float base, float subtraction) {
	return intersectSdf(base, -subtraction);
}
