#version 400 core
layout(points) in;
layout(line_strip, max_vertices = 2) out;

flat in vec3 dir[];
out vec3 colour;

void main()
{

    gl_Position = gl_in[0].gl_Position;
    colour=vec3(1,1,1);
    EmitVertex();
    gl_Position = gl_in[0].gl_Position+(vec4(dir[0],0));
    colour=vec3(1,0,0);
    EmitVertex();
    EndPrimitive();
}

