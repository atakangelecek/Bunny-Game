#version 330 core

in vec4 color;
out vec4 fragColor;

void main(void)
{
	// If overrideColor alpha is not negative, use it; otherwise, use the vertex color
    fragColor = color;
}