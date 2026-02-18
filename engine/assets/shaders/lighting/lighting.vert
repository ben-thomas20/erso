#version 410 core
// Fullscreen triangle using the gl_VertexID trick — no vertex buffer needed.
// Vertex 0: (-1,-1), Vertex 1: (3,-1), Vertex 2: (-1, 3)
out vec2 vUV;

void main()
{
    vUV         = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(vUV * 2.0 - 1.0, 0.0, 1.0);
}
