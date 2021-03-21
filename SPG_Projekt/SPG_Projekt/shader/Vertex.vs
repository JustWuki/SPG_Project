#version 430

layout (binding = 0) uniform sampler3D samp;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec3 varTexture;
out int varIndex;

const float step = 1.0 / 63.0;

void main(void) { 
    int id = gl_VertexID;
    int x = id & 0x3F;
    int y = (id >> 6) & 0x3F;
    int z = (id >> 12) & 0x3F;

    vec3 xyz = vec3(x, y, z);
    gl_Position = model * proj * view * vec4(xyz, 1.0);
    gl_PointSize = 10.0;
    /*varTexture = xyz * step;

    int b1 = int(texture(samp, varTexture).r < 0.5f);
    int b2 = int(texture(samp, varTexture + vec3(step, 0.0, 0.0)).r < 0.5f);
    int b3 = int(texture(samp, varTexture + vec3(step, 0.0, step)).r < 0.5f);
    int b4 = int(texture(samp, varTexture + vec3(0.0, 0.0, step)).r < 0.5f);
    int b5 = int(texture(samp, varTexture + vec3(0.0, step, 0.0)).r < 0.5f);
    int b6 = int(texture(samp, varTexture + vec3(step, step, 0.0)).r < 0.5f);
    int b7 = int(texture(samp, varTexture + vec3(step, step, step)).r < 0.5f);
    int b8 = int(texture(samp, varTexture + vec3(0.0, step, step)).r < 0.5f);
    varIndex = (b1 << 7) | (b2 << 6) | (b3 << 5) | (b4 << 4) | 
               (b5 << 3) | (b6 << 2) | (b7 << 1) | b8;*/
}