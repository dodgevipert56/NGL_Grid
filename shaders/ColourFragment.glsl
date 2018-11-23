#version 440 core

layout (location = 0) out vec4 fragColour; // setting the one of the 16 inputs
uniform vec4 vertColour;

void main()
{
    fragColour = vertColour; //setting the color greyscale for the teapot
}
