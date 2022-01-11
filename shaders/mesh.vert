#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;

layout(location = 0) out vec3 oFragPosition;
layout(location = 1) out vec3 oFragNormal;

layout(binding = 0) uniform ModelInfo
{
    mat4 model;
    mat4 MVP;
}
modelUbo;

void main()
{
    oFragPosition = (modelUbo.model * vec4(iPosition, 1.0)).xyz;

    gl_Position = modelUbo.MVP * vec4(iPosition, 1.0);
    oFragNormal = iNormal;
}