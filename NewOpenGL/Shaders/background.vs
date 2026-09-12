#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform vec2 textureOffset;
uniform vec4 position; // x, y, width, height (默认值应为 -1.0, -1.0, 2.0, 2.0)

void main()
{
    // 转换顶点位置以适应指定的位置和大小
    vec3 transformedPos;
    transformedPos.x = aPos.x * position.z / 2.0 + position.x + position.z / 2.0;
    transformedPos.y = aPos.y * position.w / 2.0 + position.y + position.w / 2.0;
    transformedPos.z = aPos.z;
    
    gl_Position = vec4(transformedPos, 1.0);
    
    // 调整纹理坐标实现滚动效果
    TexCoord = aTexCoord + textureOffset;
}
