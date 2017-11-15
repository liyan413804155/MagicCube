#version 330

in vec3 fragColor;

out vec4 Output;

void main ()
{
	Output = vec4(fragColor, 0);
}