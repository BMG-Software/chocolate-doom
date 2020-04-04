#version 330 core

in vec2 coordTexture;
uniform sampler2D texture;

void main()
{
  gl_FragColor = texture2D(texture, coordTexture);
}