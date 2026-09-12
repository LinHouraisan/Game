#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform float time;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;
out vec2 FlowUV;

void main() {
    //波浪位移
    float waveX = 0.1 * sin(aPos.x * 5.0 + time);
    float waveZ = 0.1 * cos(aPos.z * 5.0 + time);
    vec3 displacedPos = aPos + vec3(0.0, (waveX + waveZ) * 0.7, 0.0);
    
    //实际位置
    FragPos = vec3(model * vec4(displacedPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FlowUV = aTexCoords + vec2(time * 0.3, time * 0.1);
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0); 
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}