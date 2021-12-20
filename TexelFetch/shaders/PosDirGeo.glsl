#version 400 core
layout(points) in;
layout (triangle_strip, max_vertices = 4) out;  

in vec3 dir[];
out vec3 colour;
uniform float baseThickness=3;
uniform float pointThickness=1;
uniform vec2  viewportSize=vec2(1024,720);

void main()
{

  vec4 p1 = gl_in[0].gl_Position;
  vec4 p2= vec4((gl_in[0].gl_Position.xyz+dir[0]).xyz,p1.w);
  vec2 linedir    = normalize((p2.xy - p1.xy) * viewportSize);
  vec2 line=vec2(-linedir.y, linedir.x);
  vec2 baseOffset =  line * baseThickness / viewportSize;
  vec2 pointOffset = line * pointThickness / viewportSize;

  gl_Position = p1 + vec4(baseOffset.xy * p1.w, 0.0, 0.0);
  colour=vec3(1,0,0);
  EmitVertex();
  gl_Position = p1 - vec4(baseOffset.xy * p1.w, 0.0, 0.0);
  EmitVertex();
  colour=vec3(1,1,1);

  gl_Position = p2 + vec4(pointOffset.xy * p2.w, 0.0, 0.0);
  EmitVertex();
  gl_Position = p2 - vec4(pointOffset.xy * p2.w, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}

