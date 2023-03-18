#version 150

// vertex position and normal in world-space
in vec3 position;
in vec3 normal;
in vec4 color;

// vertex position and normal in view space, passed to fragment shader
out vec3 viewPosition;
out vec3 viewNormal;

// transformation matrix
uniform mat4 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

out vec4 col;

uniform int mode;

void main()
{
  // view-space pposition of the vertex
  vec4 viewPosition4 = modelViewMatrix * vec4(position, 1.0f);
  viewPosition = viewPosition4.xyz;
  vec3 normal = color.xyz;

  gl_Position = projectionMatrix * viewPosition4;


  viewNormal = normalize((normalMatrix * vec4(normal, 0.0f)).xyz);
  col = color;
}

