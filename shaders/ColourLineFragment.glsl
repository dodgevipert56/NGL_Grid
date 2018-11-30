#version 410 core

layout (location = 0) out vec4 fragColour; // setting the one of the 16 inputs
in vec4 colour; // if out instead of in itll be linker error

void main()
{
    fragColour = colour; //setting the color greyscale for the teapot
}
