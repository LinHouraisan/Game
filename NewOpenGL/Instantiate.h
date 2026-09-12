#pragma once

#pragma once
#include <glad/glad.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>

#include <GLFW/glfw3.h>
#include <cstdlib>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader_m.h"
#include "model_animation.h"

namespace OpenGL {

    class Instantiate {
    public:
        // 构造函数：接受一个Model对象的引用
        Instantiate(const Model& model)
            : model(model), instanceVBO(0)
        {
            // 初始化实例缓冲区
            glGenBuffers(1, &instanceVBO);
        }

        // 析构函数：释放OpenGL资源
        ~Instantiate() {
            glDeleteBuffers(1, &instanceVBO);
        }

        // 添加一个实例的变换矩阵
        void AddInstance(const glm::mat4& transform) {
            instanceTransforms.push_back(transform);
        }

        // 批量添加多个实例的变换矩阵
        void AddInstances(const std::vector<glm::mat4>& transforms) {
            instanceTransforms.insert(instanceTransforms.end(), transforms.begin(), transforms.end());
        }

        // 设置实例缓冲区并配置顶点属性指针
        void SetupInstances() {
            if (instanceTransforms.empty()) {
                std::cout << "没有实例数据需要设置。" << std::endl;
                return;
            }

            // 将实例变换矩阵数据上传到缓冲区
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, instanceTransforms.size() * sizeof(glm::mat4), &instanceTransforms[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // 为每个网格的每个顶点数组对象绑定实例缓冲区
            for (unsigned int i = 0; i < model.meshes.size(); i++) {
                unsigned int VAO = model.meshes[i].VAO;
                glBindVertexArray(VAO);

                // 设置顶点属性指针（每个变换矩阵占用4个vec4属性，位置7-10）
                glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
                std::size_t vec4Size = sizeof(glm::vec4);
                for (unsigned int j = 0; j < 4; j++) {
                    glEnableVertexAttribArray(7 + j);
                    glVertexAttribPointer(7 + j, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(j * vec4Size));
                    glVertexAttribDivisor(7 + j, 1); // 设置为1，表示每个实例使用一个变换矩阵
                }
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }
        }

        // 绘制所有实例
        void Draw(Shader& shader) {
            if (instanceTransforms.empty()) {
                std::cout << "没有实例数据需要绘制。" << std::endl;
                return;
            }
            shader.use();
            for (unsigned int i = 0; i < model.meshes.size(); i++) {
                model.meshes[i].DrawInstanced(const_cast<Shader&>(shader), instanceTransforms.size());
            }
        }

    private:
        const Model& model;
        std::vector<glm::mat4> instanceTransforms;
        unsigned int instanceVBO;
    };
}