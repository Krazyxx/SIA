#version 150

in vec3 var_normal;
in vec3 var_view;

uniform vec3 light_dir;
uniform vec3 ambient_color;
uniform int twoSided;
out vec4 out_color;

void main()
{
    vec3 normal = normalize(var_normal);
    vec3 color  = ambient_color;
    vec3 view = normalize(var_view);
    vec3 h = normalize(view+light_dir);

    float diffuse;
    if(twoSided == 1)
        diffuse = abs(dot(normal,light_dir));
    else
        diffuse = clamp(dot(normal,light_dir),0,1);
    out_color = vec4(vec3(0.02 + color*diffuse), 1);
}
