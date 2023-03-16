#version 150

in vec4 col;
in vec2 TexCoord;

out vec4 c;
uniform sampler2D textureSampler;

void main()
{
  // compute the final pixel color
  c = texture(textureSampler, TexCoord);
}

