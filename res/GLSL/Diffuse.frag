#version 330 core

in vec3 fragCoord;
in vec3 normalDir;

out vec4 color;

uniform vec3 _Color;
uniform vec3 _LightPos;
uniform int _LightCount; // point light count

struct PointLight {
	vec3 color;
	vec3 position;
	float kc; // attenuation constant term
	float kl; // attenuation linear term
	float kq; // attenuation quadratic term
};

struct DirLight {
	vec3 color;
	vec3 direction;
};

uniform DirLight _Sun;

// Maximum number of lights
// _LightCount <= POINT_LIGHT_COUNT
#define POINT_LIGHT_COUNT 16
uniform PointLight _Lights[POINT_LIGHT_COUNT];

vec3 pointLight(int index)
{
	vec3 ambient = vec3(0.25f, 0.25f, 0.25f);

	vec3 lightDir = normalize(_Lights[index].position - fragCoord);
    // diffuse shading
    float diff = max(dot(normalDir, lightDir), 0.0f);
	float plDistance = length(_Lights[index].position - fragCoord);
	float attenuation = 1.0f/(_Lights[index].kc + _Lights[index].kl*plDistance + _Lights[index].kq*(plDistance*plDistance));

	vec3 diffuse = vec3(_Lights[index].color * diff) * attenuation;
	ambient *= attenuation;


	return ambient + diffuse;
}

vec3 sunLight()
{
	return _Sun.color * max(dot(normalDir, normalize(-_Sun.direction)), 0.0f);
}

void main()
{
	float kc = 1.0f;
	float kl = 0.09f;
	float kq = 0.032f;

	vec3 pointLights = vec3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < _LightCount; i++)
	{
		pointLights += pointLight(i);
	}

	vec3 ambient = vec3(0.25f, 0.25f, 0.25f);
	//vec3 lightDir = normalize(_LightPos - fragCoord);
	color = vec4(_Color * (sunLight() + pointLights + ambient), 1.0f);
}