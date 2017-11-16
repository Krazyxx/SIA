#version 330 core

uniform int wireframe;

uniform vec3 CoolColor;
uniform vec3 WarmColor;
uniform sampler2D colormap;
uniform float tex_blend_factor;

in vec3 vert_normal_view;
in vec3 vert_color;
in vec2 vert_uv;

out vec4 out_color;

vec3 light_dir = normalize(vec3(1,1,0.8));

vec3 coldwarm_shading(vec3 N, vec3 L, vec3 color){

  float NdotL = dot(N,L);
  vec3 kcool = CoolColor + (0.4 * color);
  vec3 kwarm = WarmColor + (0.6 * color);
  return mix(kcool, kwarm, NdotL);
}

void main(void)
{
  vec3 tex_color = texture(colormap, vert_uv).xyz;
  vec3 color = mix(vert_color,tex_color,tex_blend_factor);
  color = coldwarm_shading(normalize(vert_normal_view), light_dir, color);
  if(wireframe==1)
    color = color*0.7;
  out_color = vec4(color,1);
}
