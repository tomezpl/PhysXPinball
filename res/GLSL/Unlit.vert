#version 330 core

layout(location =0) in vec3 position;
layout(location =1) in vec3 normal;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Proj;

uniform mat4 _MVP;

void main()
{
	gl_Position = _Proj * _View * _Model * vec4(position, 1.0f);
	//gl_Position = _MVP * vec4(position, 1.0f);
}