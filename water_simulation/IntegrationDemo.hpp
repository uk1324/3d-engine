#pragma once

struct IntegrationDemo {
	IntegrationDemo();

	void update();

	int simpsonsMethodStepCount = 6;
	void simpsonsMethod();

	double f(double x);

	float start = 0.0f;
	float end = 10.0f;
};