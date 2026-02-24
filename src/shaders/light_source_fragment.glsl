#version 330 core
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D diffuseTexture;
uniform vec3 lightSource;

void main() {
	FragColor = texture(diffuseTexture, TexCoords) * lightSource * vec4(1.0);
}
