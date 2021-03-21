#version 430

in vec3 varPosition;

out float noise;

void main(void) {
    float f1 = sin(varPosition.x * 5.12);
    float f2 = cos(varPosition.y * 7);
    float f3 = sin(varPosition.z * 9);
    noise = (f1 * f1 * f1 + f2 * f2 + f3 * f3) * (1.0f / 3.0f);
}