#version 430 core
out vec4 fragColor;

/*generated end*/

in vec3 normal;

uniform sampler2DRect TempTex;

void main() {
	fragColor = texture(TempTex, gl_FragCoord.xy);
}
