#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in int inIndex;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform ModelInfo
{
    mat4 model;
    mat4 MVP;
}
modelUbo;

layout(binding = 3) uniform PointSize
{
    uint size;
}
pointSizeUbo;

void main()
{
    gl_PointSize = pointSizeUbo.size;
    gl_Position = modelUbo.MVP * vec4(inPosition, 1.0);
    fragColor = inColor;
}