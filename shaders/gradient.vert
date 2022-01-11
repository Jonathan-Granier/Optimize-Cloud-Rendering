#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;

void main()
{
    gl_Position = vec4(iPosition, 1.0);
}