#version 150

// interpolated from vertex program outpus
in vec3 viewPosition;
in vec3 viewNormal;
in vec2 TexCoord;

// output color
out vec4 c;
uniform sampler2D textureSampler;

// Phong Lighting, properties of the directional light
uniform vec4 La; // light ambient
uniform vec4 Ld; // light diffuse
uniform vec4 Ls; // light specular
uniform vec3 viewLightDirection;

// mesh optical properties
uniform vec4 ka; // mesh ambient
uniform vec4 kd; // mesh diffuse
uniform vec4 ks; // mesh specular
uniform float alpha; // shininess

in vec3 color;

void main()
{
  // camera is at (0,0,0) after the modelview transformation
  vec3 eyedir = normalize(vec3(0,0,0) - viewPosition);
  
  // reflected light direction
  vec3 reflectDir = -reflect(viewLightDirection, viewNormal);

  // Phong lighting
  float d = max(dot(viewLightDirection, viewNormal), 0.0f);
  float s = max(dot(reflectDir, eyedir), 0.0f);

  // compute the final color
  vec4 light = ka * La + d * kd * Ld + pow(s, alpha) * ks * Ls;
  vec4 t = texture(textureSampler, TexCoord);
  c = vec4(light.x * t.x, light.y * t.y, light.z * t.z, 1.0f);
}
