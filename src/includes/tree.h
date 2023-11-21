#ifndef TREE_H
#define TREE_H

#include <iostream>
#include <algorithm>
#include <vector>
#include <deque>
#include <memory>

namespace Tree {
    template <typename T>
    struct Node {
        bool isLeaf;
        Node<T>* parent;
        std::deque<T> keys;
        std::deque<Node<T>*> children;
        Node<T>* next;
        Node<T>* prev;
        Node(bool leaf) : isLeaf(leaf), parent(nullptr), next(nullptr), prev(nullptr) {};
        void rebuild();
        void checkParentChildPointers();
    };


    template<typename T>
    class BPlusTree {
        private:
            Node<T>* rootPtr;
            int ORDER_;

        public:
            BPlusTree(int order);
            ~BPlusTree();

            // Public API
            Node<T>* getRoot();
            void insert(T key);
            void remove(T key);
            std::optional<T> get(T key);
            void printBPlusTree();

        private:
            // Private helper functions
            Node<T>* findLeafNode(Node<T>* node, T key);
            void splitNode(Node<T>* node, T key);
            void insertNonFull(Node<T>* node, T key);
            bool removeFromLeaf(Node<T>* node, T key);

            bool isHalfFull(Node<T>* node);
            bool moreHalfFull(Node<T>* node);

            void removeBorrow(Node<T>* node);
            void removeMerge(Node<T>* node);
            void updateKeyToLCA(Node<T>* left, Node<T>* right, bool isLeftToRight);
    };
}

#endif
