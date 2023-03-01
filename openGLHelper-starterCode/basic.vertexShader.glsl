#version 150

in vec3 position;
in vec4 color;
out vec4 col;

in float p_left, p_right, p_up, p_down;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

// 0 is regular mode and 1 is smooth mode
uniform int mode;

const float eps = 1e-5f; 

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)

  // 0 for standard mode, 1 for smooth mode
  switch(mode) {
    case 0:
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
      col = color;
      break;
    case 1:
      float smoothened_height = (p_left + p_right + p_up + p_down) / 4.0;

      // 50 here is a paddding to prevent division by a very small number which results in white spike
      vec4 output_color = smoothened_height * max(color, vec4(eps)) / max(position.y, eps * 50);
      output_color.a = 1.0f;
      vec3 smooth_position = position;
      smooth_position.y = smoothened_height;
      gl_Position = projectionMatrix * modelViewMatrix * vec4(smooth_position, 1.0f);
      col = output_color;
      break;
    default:
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
      col = color;
      break;
  }
}

