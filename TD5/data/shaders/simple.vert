#version 410 core

uniform mat4 projection_matrix;
uniform mat4 model_matrix;
uniform mat4 view_matrix;

in vec3 vtx_position;
in vec3 vtx_color;

void main()
{
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(vtx_position, 1.);
}
