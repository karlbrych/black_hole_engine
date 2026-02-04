#version 300 es
precision mediump float;

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D bgTex;

void main() {
  FragColor = texture(bgTex, vUV);
}
