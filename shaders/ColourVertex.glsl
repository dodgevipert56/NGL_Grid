#version 440 core

layout (location = 0) in vec3 inVert; //you tell which direction the input is coming from
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
//uniform mat4 MVP = mat4(1); //setting identity to 1
uniform mat4 MVP;

void main()
{
    //mat4 tx = mat4(0.2*inNormal, 0, 0, ); // diagonal matrix setting
    gl_Position = MVP*vec4(inVert, 1.0); // global variable passed out to the pos
}
