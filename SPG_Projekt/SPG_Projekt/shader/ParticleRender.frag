#version 430

layout (binding = 0) uniform sampler2D uSampler;

in vec2 vTexCoord;
in vec4 vColorPart;

out vec4 oColor;

void main()
{
  vec4 vTexColor = texture2D(uSampler, vTexCoord);
  oColor = vTexColor * vColorPart;
}