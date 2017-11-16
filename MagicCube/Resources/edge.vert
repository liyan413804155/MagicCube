#version 330

in vec3 vertex;

uniform mat4 projView;
uniform mat4 model;

void main()
{
	gl_Position = projView * model * vec4(vertex, 1);
}