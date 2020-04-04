#version 330 core

attribute vec3 coord3d;
attribute vec2 coordTex;
uniform mat4 mvp;

out vec2 coordTexture;

void main()
{
  gl_Position = mvp * vec4(coord3d, 1.0);
  coordTexture = coordTex;
}