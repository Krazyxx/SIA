#version 150

in vec3 vtx_position;

uniform mat4 mat_obj;
uniform mat4 mat_view;
uniform mat4 mat_proj;

void main()
{
  gl_Position =  mat_proj * mat_view * mat_obj * vec4(vtx_position,1);
}
