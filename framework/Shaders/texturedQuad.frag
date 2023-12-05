#version 430 core

in vec2 fragTexturePosition; 
out vec4 fragColor;

/*generated end*/

uniform sampler2D renderedTexture;

void main() {
	//fragColor = vec4(vec3(1.0, 1.0, 0.0), 1.0);
	fragColor = texture(renderedTexture, fragTexturePosition);
	//fragColor = vec4(fragTexturePosition, 0.0, 1.0);
}
