#version 330

in vec3 vertex;

uniform mat4 projViewModel;

void main()
{
	gl_Position = projViewModel * vec4(vertex, 1);
}