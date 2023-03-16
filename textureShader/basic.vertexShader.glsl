#version 150

in vec3 position;
in vec4 color;
in vec2 texCoord;

out vec2 TexCoord;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

// 0 is regular mode and 1 is smooth mode
uniform int mode;


void main()
{
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  col = color;
  TexCoord = texCoord;
}

