#version 330 core

out vec4 color;

uniform vec3 _Color;

void main()
{
	color = vec4(_Color, 1.0f);
}