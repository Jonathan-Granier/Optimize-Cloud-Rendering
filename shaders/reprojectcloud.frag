#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) flat in int vertexIndex;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outIndex;

void main()
{
    //Invalid point
    if (vertexIndex == -1)
    {
        discard;
    }
    outColor = vec4((fragColor), 1.0);
    outIndex = vertexIndex;
}