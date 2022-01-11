#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out int vertexIndex;

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

vec3 UIntToVec3(uint i)
{
    vec4 v = unpackUnorm4x8(i);
    return vec3(v.r, v.g, v.b);
}

void main()
{

    gl_PointSize = pointSizeUbo.size;
    gl_Position = modelUbo.MVP * vec4(inPosition, 1.0);
    fragColor = UIntToVec3(inColor);
    vertexIndex = gl_VertexIndex;
}