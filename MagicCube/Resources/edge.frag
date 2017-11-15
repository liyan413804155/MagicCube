#version 330

uniform vec3 edgeColor;

out vec4 Output;

void main ()
{
	Output = vec4(edgeColor, 1);
}