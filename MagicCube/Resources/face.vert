#version 330

in vec3 vertex;
in vec3 normal;
in vec3 color;

uniform mat4 projView;
uniform mat4 model;

out vec3 fragColor;

void main()
{
	gl_Position = projView * model * vec4(vertex, 1);
	fragColor = color;
}