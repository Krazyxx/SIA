#version 330
in vec2 position;
out vec2 uv;

void main() 
{
    gl_Position = vec4(position.x*2-1, position.y*2-1, 0.0, 1.0);
    uv = vec2(position.x, 1-position.y);
}
