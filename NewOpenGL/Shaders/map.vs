#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;  //光源视角下的变换矩阵

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;  //光源视角下的顶点位置

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0)); 
    Normal = mat3(transpose(inverse(model))) * norm; 
    TexCoords = tex;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);  //计算光源视角下的位置
    gl_Position = projection * view * vec4(FragPos, 1.0);
}