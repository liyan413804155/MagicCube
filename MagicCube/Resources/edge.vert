#version 330

in vec3 vertex;

uniform mat4 projView;

void main()
{
	gl_Position = projView * vec4(vertex, 1);
}