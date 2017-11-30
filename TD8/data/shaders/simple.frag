#version 330 core

uniform vec3 col;

out vec4 out_color;

void main()
{
        out_color = vec4(col,1.f);
}
