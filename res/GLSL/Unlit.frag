#version 330 core

out vec4 color;

uniform vec3 _Color;
uniform vec3 _LightPos; // unused here but declared to avoid problems in C++ code

void main()
{
	color = vec4(_Color, 1.0f);
}