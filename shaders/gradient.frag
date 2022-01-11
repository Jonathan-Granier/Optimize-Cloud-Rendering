#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform ScreenSize
{
    uint Width;
    uint Height;
}
screenSize;

void main()
{
    float grad = gl_FragCoord.y / float(screenSize.Height);
    float r = (grad * 0.13);
    float g = (grad * 0.14);
    float b = (grad * 0.19);
    outColor = vec4(r, g, b, 0);
}