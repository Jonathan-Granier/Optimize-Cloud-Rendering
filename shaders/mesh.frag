#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 iFragPosition;
layout(location = 1) in vec3 iFragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform CameraInfo
{
    mat4 view;
    mat4 invView;
    mat4 proj;
    mat4 invProj;
    vec3 camPos;
}
cameraUbo;

layout(binding = 2) uniform LightingInfo
{
    vec3 color;
    float intensity;
    bool threePointEnabled;
}
lightingUbo;

void main()
{
    vec3 normal = normalize(iFragNormal);

    vec3 fullViewDir = cameraUbo.camPos - iFragPosition;
    vec3 viewDir = normalize(fullViewDir);

    vec3 color = vec3(1.0);

    if (lightingUbo.threePointEnabled)
    {
        vec3 diffuse = vec3(0.0);
        vec3 specular = vec3(0.0);

        mat3 invViewMat = mat3(cameraUbo.invView);

        // Create other light sources based on the vertices' position
        float camDist = length(fullViewDir);

        // Key light
        //vec3 keyLightPos = invViewMat * normalize(vec3(-1.0, 1.0, 0.0));
        vec3 keyLightPos = (cameraUbo.invView * vec4(normalize(vec3(-1.0, 1.0, 0.0)), 1.0)).xyz; // Or the invViewMat mat3?
        vec3 keyLightDir = normalize(vec3(1.0, 1.0, -1.0));
        vec3 keyHalfDir = normalize(keyLightDir + viewDir);
        diffuse += max(dot(keyHalfDir, normal), 0.0) * lightingUbo.intensity;
        specular += pow(max(dot(keyHalfDir, normal), 0.0), 32.0) * lightingUbo.color * lightingUbo.intensity;

        // Fill light
        vec3 fillLightPos = (cameraUbo.invView * vec4(normalize(vec3(1.0, 0.0, 0.0)), 1.0)).xyz;
        vec3 fillLightDir = normalize(vec3(-1.0, 0.0, -1.0));
        vec3 fillHalfDir = normalize(fillLightDir + viewDir);
        diffuse += max(dot(fillHalfDir, normal), 0.0) * lightingUbo.intensity;
        specular += pow(max(dot(fillHalfDir, normal), 0.0), 32.0) * lightingUbo.color * lightingUbo.intensity;

        // Back light
        vec3 backLightPos = (cameraUbo.invView * vec4(normalize(vec3(0.0, -camDist, -camDist * 2.0)), 1.0)).xyz;
        vec3 backLightDir = normalize(vec3(0.0, camDist, camDist * 2.0));
        vec3 backHalfDir = normalize(backLightDir + viewDir);
        diffuse += max(dot(backHalfDir, normal), 0.0) * lightingUbo.intensity;
        specular += pow(max(dot(backHalfDir, normal), 0.0), 32.0) * lightingUbo.color * lightingUbo.intensity;

        color = diffuse * 0.2 + specular;
    }
    else
    {
        // Basic lighting from the camera's position
        color = max(dot(viewDir, normal), 0.0) * lightingUbo.color * lightingUbo.intensity;
    }

    outColor = vec4(color, 1.0);
}