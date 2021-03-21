#version 430

layout (binding = 0) uniform sampler3D samp;

in vec3 varTextureG;

out vec4 color;

void main(void) {
    float f = texture(samp, varTextureG).r;
    color = vec4(f, f, f, 1.0);
}