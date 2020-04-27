#version 330 core

in vec3 fragCoord;

out vec4 color;

void main()
{
	vec3 sparkColor = vec3(1.0, 215.0/255.0, 0.0);
	color = vec4(sparkColor, 1.0);
	gl_FragDepth = 0.0;
}