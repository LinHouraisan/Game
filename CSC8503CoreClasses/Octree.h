#pragma once

namespace NCL
{
	using namespace NCL::Maths;
	namespace CSC8503
	{
        // 前向声明 Octree 类
        template<class T>
        class Octree;

        // OctreeEntry 结构体，用于存储八叉树节点中的对象及其位置和大小
        template<class T>
        struct OctreeEntry {
            Vector3 pos;   // 对象的位置
            Vector3 size;  // 对象的大小
            T object;      // 对象本身

            // 默认构造函数
            OctreeEntry() {}

            // 构造函数
            OctreeEntry(T obj, Vector3 pos, Vector3 size) {
                object = obj;
                this->pos = pos;
                this->size = size;
            }
        };

        // OctreeNode 类，表示八叉树的一个节点
        template<class T>
        class OctreeNode {
        public:
            typedef std::function<void(std::list<OctreeEntry<T>>&)> OctreeFunc;

        protected:
            friend class Octree<T>;

            OctreeNode() {}

            // 构造函数，初始化节点的位置和大小
            OctreeNode(Vector3 pos, Vector3 size) {
                children = nullptr;
                this->position = pos;
                this->size = size;
            }

            // 析构函数，删除子节点
            ~OctreeNode() {
                delete[] children;
            }

            // 插入对象到八叉树节点中
            void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize) {
                // 如果当前节点没有子节点
                if (!children) {
                    // 如果当前节点的内容数量小于最大容量或已达到最大深度
                    if (contents.size() < maxSize || depthLeft == 0) {
                        // 将对象插入当前节点
                        contents.push_back(OctreeEntry<T>(object, objectPos, objectSize));
                        return;
                    }
                    else {
                        // 否则分裂当前节点
                        Split();
                        // 重新分配内容到子节点
                        for (auto& entry : contents) {
                            Vector3 midpoint = position + (size * 0.5f);
                            int childIndex = (entry.pos.x > midpoint.x) |
                                ((entry.pos.y > midpoint.y) << 1) |
                                ((entry.pos.z > midpoint.z) << 2);
                            children[childIndex].Insert(entry.object, entry.pos, entry.size, depthLeft - 1, maxSize);
                        }
                        contents.clear();
                    }
                }

                // 计算对象应插入的子节点索引
                Vector3 midpoint = position + (size * 0.5f);
                int childIndex = (objectPos.x > midpoint.x) |
                    ((objectPos.y > midpoint.y) << 1) |
                    ((objectPos.z > midpoint.z) << 2);

                // 将对象插入到对应的子节点
                children[childIndex].Insert(object, objectPos, objectSize, depthLeft - 1, maxSize);
            }

            // 分裂当前节点为八个子节点
            void Split() {
                Vector3 halfSize = size * 0.5f;
                children = new OctreeNode<T>[8];
                for (int i = 0; i < 8; ++i) {
                    Vector3 newPos = position;
                    if (i & 1) newPos.x += halfSize.x;
                    if (i & 2) newPos.y += halfSize.y;
                    if (i & 4) newPos.z += halfSize.z;
                    children[i] = OctreeNode<T>(newPos, halfSize);
                }
            }

            // 调试绘制八叉树节点
            void DebugDraw() {
                // 添加绘制代码以在场景中可视化八叉树节点
            }

            // 对节点内容执行操作
            void OperateOnContents(OctreeFunc& func) {
                // 如果有子节点，递归对子节点执行操作
                if (children) {
                    for (int i = 0; i < 8; ++i) {
                        children[i].OperateOnContents(func);
                    }
                }
                // 否则对当前节点的内容执行操作
                else {
                    if (!contents.empty()) {
                        func(contents);
                    }
                }
            }

            // 查找特定对象
            bool Find(const T& object, OctreeEntry<T>& result) {
                // 在当前节点的内容中查找对象
                for (auto& entry : contents) {
                    if (entry.object == object) {
                        result = entry;
                        return true;
                    }
                }

                // 如果有子节点，递归查找子节点
                if (children) {
                    for (int i = 0; i < 8; ++i) {
                        if (children[i].Find(object, result)) {
                            return true;
                        }
                    }
                }

                return false;
            }

            bool Remove(const T& object, int depthLeft, int maxSize) {
                if (!children) {
                    for (auto it = contents.begin(); it != contents.end(); ++it) {
                        if (it->object == object) {
                            contents.erase(it);
                            return true;
                        }
                    }
                    return false;
                }

                OctreeEntry<T> entry;
                if (Find(object, entry)) {
                    Vector3 objectPos = entry.pos;
                    Vector3 objectSize = entry.size;

                    Vector3 midpoint = position + (size * 0.5f);
                    int childIndex = (objectPos.x > midpoint.x) |
                        ((objectPos.y > midpoint.y) << 1) |
                        ((objectPos.z > midpoint.z) << 2);

                    bool removed = children[childIndex].Remove(object, depthLeft - 1, maxSize);

                    if (removed) {
                        int totalContents = 0;
                        for (int i = 0; i < 8; ++i) {
                            totalContents += children[i].contents.size();
                        }
                        if (totalContents <= maxSize) {
                            for (int i = 0; i < 8; ++i) {
                                contents.insert(contents.end(), children[i].contents.begin(), children[i].contents.end());
                                children[i].contents.clear();
                            }
                            delete[] children;
                            children = nullptr;
                        }
                    }

                    return removed;
                }

                return false;
            }

        protected:
            std::list<OctreeEntry<T>> contents; // 节点中的内容列表
            Vector3 position;                   // 节点的位置
            Vector3 size;                       // 节点的大小
            OctreeNode<T>* children;            // 子节点数组
        };

        // Octree 类，表示整个八叉树
        template<class T>
        class Octree {
        public:
            // 构造函数，初始化八叉树根节点、最大深度和最大容量
            Octree(Vector3 size, int maxDepth = 6, int maxSize = 5)
                : root(Vector3(), size), maxDepth(maxDepth), maxSize(maxSize) {}

            // 析构函数
            ~Octree() {}

            // 插入对象到八叉树中
            void Insert(T object, const Vector3& pos, const Vector3& size) {
                root.Insert(object, pos, size, maxDepth, maxSize);
            }

            // 调试绘制八叉树
            void DebugDraw() {
                root.DebugDraw();
            }

            // 对八叉树内容执行操作
            void OperateOnContents(typename OctreeNode<T>::OctreeFunc func) {
                root.OperateOnContents(func);
            }

            // 查找特定对象
            bool Find(const T& object, OctreeEntry<T>& result) {
                return root.Find(object, result);
            }

            bool Remove(const T& object) {
                return root.Remove(object, maxDepth, maxSize);
            }

        protected:
            OctreeNode<T> root; // 八叉树的根节点
            int maxDepth;       // 八叉树的最大深度
            int maxSize;        // 每个节点的最大容量
        };
	}
}
