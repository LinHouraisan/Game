#pragma once
#include <vector>

namespace NCL
{
	namespace CSC8503
	{
        template<class T>
        class ObjectPool {
        public:
            ObjectPool(size_t initialSize, size_t maxSize) : maxSize(maxSize) {
                for (size_t i = 0; i < initialSize; ++i) {
                    Release(new T());
                }
            }

            ~ObjectPool() {
                for (auto obj : allObjects) {
                    delete obj;
                }
            }

            T* Acquire() {
                if (freeObjects.empty()) {
                    T* newObj = new T();
                    allObjects.push_back(newObj);
                    return newObj;
                }
                T* obj = freeObjects.back();
                freeObjects.pop_back();
                return obj;
            }

            void Release(T* obj) {
                if (freeObjects.size() < maxSize) {
                    freeObjects.push_back(obj);
                } else {
                    delete obj;
                }
            }

        private:
            std::vector<T*> freeObjects;
            std::vector<T*> allObjects;
            size_t maxSize; // 对象池的最大容量
        };
	}
}
