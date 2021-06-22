#version 430 core

layout (vertices = 3) out;

uniform float tessellation_factor;

void main()
{
    if (gl_InvocationID == 0)
    {
        gl_TessLevelInner[0] = tessellation_factor;
        gl_TessLevelOuter[0] = tessellation_factor;
        gl_TessLevelOuter[1] = tessellation_factor;
        gl_TessLevelOuter[2] = tessellation_factor;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}