#version 430 core

in vec2 position; 

in vec4 color; 
in float smoothing; 
in float lineWidth; 
in float lineLength; 
out vec4 fragColor;

/*generated end*/

void main() {
	float len = lineLength;
	float x = position.x * len;
	float d;
	if (x < lineWidth || x > len - lineWidth) {
		float clampedX = clamp(x, lineWidth, len - lineWidth);
		d = distance(vec2(x, position.y * lineWidth), vec2(clampedX, 0.0));
	} else {
		d = abs(position.y) * lineWidth;
	}
	d /= lineWidth;
	d -= 1.0 - smoothing;
	d = smoothstep(smoothing, 0.0, d);

	fragColor = vec4(color.rgb, d);
}
