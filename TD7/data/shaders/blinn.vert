#version 330 core

uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat4 obj_mat;
uniform mat3 normal_mat;
uniform int disp_selection;

in vec3 vtx_position;
in vec3 vtx_color;
in vec3 vtx_normal;
in vec2 vtx_texcoord;
in int vtx_mask;

out vec3 vert_normal_view;
out vec3 vert_color;
out vec2 vert_uv;

void main()
{
  vert_uv  = vtx_texcoord;
  gl_Position = proj_mat * view_mat * obj_mat * vec4(vtx_position,1.0);

  vert_normal_view = normalize(normal_mat * vtx_normal);

  if(bool(disp_selection) && (vtx_mask!=0))
    vert_color = vec3(1,0.1,0.1) + vtx_color*0.2;
  else
    vert_color = vtx_color*0.6;
}
