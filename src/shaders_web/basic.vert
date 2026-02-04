#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTex;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 vTex;

void main() {
  vTex = aTex;
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
