#version 330 core

attribute vec2 coord3d;

void main()
{
  gl_Position = vec4(coord3d, 1.0, 1.0);
}