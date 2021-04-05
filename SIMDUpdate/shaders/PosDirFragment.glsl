#version 400

uniform vec4 Colour;
layout(location=0) out vec4 fragColour;
in vec3 colour;

void main()
{
    fragColour.rgb=colour;
}