#version 330 core

uniform vec3 light_col;

out vec4 out_color;

void main()
{
	out_color = vec4(light_col,1.f);
}
