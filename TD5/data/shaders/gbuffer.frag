#version 410 core

uniform float specular_coef;

in vec3 vert_normal_view;
in vec3 vert_color;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal;

void main()
{
    out_color = vec4(vert_color, specular_coef);
    out_normal = vec4((vert_normal_view + 1.) / 2., gl_FragCoord.z);
}
