#version 450

layout(set = 0, binding = 0) uniform UniformBufferObj
{
    mat4 model;
    mat4 view;
    mat4 proj;

    vec3 direction;
    vec3 color;
    float intensity;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main()
{
    //���C�g�̕����𐳋K��
    vec3 normalizedLightDir = normalize(ubo.direction);
    vec3 normalizedNormal = normalize(fragNormal);

    //Diffuse���C�e�B���O�v�Z
    float diffuse = max(dot(normalizedNormal, normalizedLightDir), 0.0);
    vec3 diffuseColor = diffuse * ubo.color * ubo.intensity;

    //�e�N�X�`���̐F���擾
    vec4 textureColor = texture(texSampler, fragTexCoord);

    //�e�N�X�`���ƃ��C�g�̐F���|�����킹��
    vec3 finalColor = textureColor.rgb * diffuseColor;

    //�ŏI�I�ȃJ���[���o�� (�e�N�X�`���̃A���t�@�l���g�p)
    outColor = vec4(finalColor, textureColor.a);
}