#version 330 core

in vec3 fragCoord;
in vec3 normalDir;

out vec4 color;

uniform vec3 _Color;

void main()
{
	vec3 lightPos = vec3(0.0f, 5.0f, 5.0f);
	vec3 lightDir = normalize(lightPos - fragCoord);
	float diffuse = max(dot(normalize(normalDir), lightDir), 0.0f);
	color = vec4(_Color * diffuse, 1.0f);
}