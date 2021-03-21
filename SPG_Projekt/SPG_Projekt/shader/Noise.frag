#version 430

in vec3 varPosition;

out float noise;

void main(void) {
    float sinX = sin(varPosition.x * 5.8905);
    float cosY = cos(varPosition.y * 5.8905);
    float cosZ = cos(varPosition.z * 5.8905);
    noise = (sinX * sinX + cosY * cosY + cosZ * cosZ) * (1.0f / 3.0f);
}