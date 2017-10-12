#version 330

uniform sampler2D source;
uniform float scale;
uniform int srgb;
in vec2 uv;
out vec4 out_color;

float toSRGB(float value) {
    if (value < 0.0031308)
        return 12.92 * value;
    return 1.055 * pow(value, 0.41666) - 0.055;
}

void main() 
{
    vec4 color = texture(source, uv);
	color *= scale / color.w;
    if(srgb == 1)
       out_color = vec4(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b), 1);
    else
       out_color = vec4(color.rgb,1);
}
