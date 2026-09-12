#pragma once
#include <vector>
#include <string>
#include "model_animation.h"
#include "animator.h"
#include "stb_image.h"

namespace OpenGL {
    class GameMaterialLoad {
    public:

        static unsigned int LoadTexture(const char* path);
        static unsigned int LoadCubemap(const std::vector<std::string>& faces);
        static Model* LoadModel(const std::string& path);
        static Animation* LoadAnimation(const std::string& path, Model* model);
        static std::vector<std::vector<int>> LoadMap(const std::string& filename);
    };
}