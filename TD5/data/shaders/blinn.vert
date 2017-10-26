#version 410 core

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat3 normal_matrix;

in vec3 vtx_position;
in vec3 vtx_color;
in vec3 vtx_normal;

out vec4 vert_pos_view;
out vec3 vert_normal_view;
out vec3 vert_color;

void main()
{
    vert_pos_view  = view_matrix * model_matrix * vec4(vtx_position,1.0);

    gl_Position = projection_matrix * vert_pos_view;

    vert_normal_view = normalize(normal_matrix * vtx_normal);

    vert_color = vtx_color;
}
