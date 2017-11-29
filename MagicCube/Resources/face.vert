#version 330

in vec3 vertex;
in vec3 normal;
in vec3 color;

uniform mat4 proj;
uniform mat4 viewModel;

uniform float ambient;
uniform float diffuse;
uniform float shininess;
uniform float specular;

out vec3 fragColor;

void main()
{
	gl_Position = proj * viewModel * vec4(vertex, 1);
	
	vec3 eyeNormal = normalize((viewModel * vec4(normal, 0)).xyz);
	
	float cosAngle = min (max (dot(eyeNormal, vec3(0, 0, 1)), 0), 1);
	
	fragColor = vec3(1) * ambient+ color * (diffuse * cosAngle + specular * pow(cosAngle, shininess));

	fragColor = min(vec3(1), fragColor);
}