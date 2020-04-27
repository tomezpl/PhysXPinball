#version 330 core

// Spark Explosion particle effect
// Vertex Shader

layout(location = 0) in vec3 position;

uniform mat4 _Model;
uniform mat4 _View;
uniform mat4 _Proj;

out vec3 fragCoord;
out vec3 center;

void main()
{
	gl_Position = _Proj * _View * _Model * vec4(position, 1.0f);
	fragCoord = vec4(_Model * vec4(position, 1.0f)).xyz;
}