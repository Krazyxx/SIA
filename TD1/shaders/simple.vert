#version 150

in vec3 vtx_position;
in vec3 vtx_normal;

uniform mat4 mat_obj;
uniform mat4 mat_view;
uniform mat4 mat_proj;
uniform mat3 mat_normal;
uniform vec3 cam_pos;

out vec3 var_normal;
out vec3 var_view;

void main()
{
  var_normal = normalize(mat_normal * vtx_normal);
  var_view = normalize(cam_pos - (mat_obj * vec4(vtx_position,1)).xyz);
  gl_Position =  mat_proj * mat_view *  mat_obj * vec4(vtx_position,1);
}
