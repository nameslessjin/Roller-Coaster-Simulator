#version 150

in vec3 position;
in vec2 texCoord;
out vec2 TexCoord;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

// 0 is regular mode and 1 is smooth mode
uniform int mode;

const float eps = 1e-5f; 

void main()
{
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  TexCoord = texCoord;
}

