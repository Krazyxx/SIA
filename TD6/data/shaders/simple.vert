#version 330 core

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

in vec4 vtx_position;

void main()
{
        gl_Position = projection_matrix * view_matrix * model_matrix * vtx_position;
}
