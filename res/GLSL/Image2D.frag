#version 330 core

uniform sampler2D _Image;

in vec2 uv;

out vec4 color;

void main()
{
	color = vec4(texture2D(_Image, uv));
	gl_FragDepth = 0.0;
}