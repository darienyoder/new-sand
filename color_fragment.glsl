#version 330 core

layout(origin_upper_left) in vec4 gl_FragCoord;
out vec4 clr;

uniform vec3 color;

void main()
{
	clr.rgb = color;
}