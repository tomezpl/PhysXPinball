#version 330 core

in vec3 fragCoord;

uniform float _Opacity = 1.0;

out vec4 color;

void main()
{
	vec3 sparkColor = vec3(1.0, 215.0/255.0, 0.0);
	color = vec4(sparkColor, _Opacity);
	//gl_FragDepth = 0.0;
}