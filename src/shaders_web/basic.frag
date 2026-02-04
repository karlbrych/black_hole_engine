#version 300 es
precision mediump float;

in vec2 vTex;
out vec4 FragColor;

uniform sampler2D tex0;

void main() {
  FragColor = texture(tex0, vTex);
}
