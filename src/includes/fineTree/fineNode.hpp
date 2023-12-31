#pragma once
#include <cassert>
#include <optional>
#include "../tree.h"

namespace Tree {
    template <typename T>
    void FineNode<T>::releaseAll() {
        if (!isLeaf) {
            for (auto child : children) child->releaseAll();
        }
        delete this;
    }

    template <typename T>
    T getMin(FineNode<T>* node) {
        while (!node->isLeaf) node = node->children[0];
        return node->keys[0];
    }

    template <typename T>
    bool FineNode<T>::debug_checkParentPointers() {
        for (Tree::FineNode<T>* child : children) {
            if (child->parent != this) 
                return false;
            if (!(child->isLeaf || child->debug_checkParentPointers())) 
                return false;
        }

        return  !(this->parent != nullptr && this->parent->children[childIndex] != this);
    }
    
    template <typename T>
    void FineNode<T>::printKeys() {
        std::cout << "[";
        for (int i = 0; i < numKeys(); i ++) {
            std::cout << keys[i];
            if (i != numKeys() - 1) std::cout << ",";
        }
        std::cout << "]";
    }

    template <typename T>
    bool FineNode<T>::debug_checkOrdering(std::optional<T> lower, std::optional<T> upper) {
        for (const auto key : this->keys) {
            if (lower.has_value() && key < lower.value()) {                
                return false;
            }
            if (upper.has_value() && key >= upper.value()) {
                return false;
            }
        }
        
        if (!this->isLeaf) {
            for (int i = 0; i < numChild(); i ++) {\
                if (i == 0) {
                    if (!this->children[i]->debug_checkOrdering(lower, this->keys[0])) 
                        return false;
                } else if (i == numChild() - 1) {
                    if (!this->children[i]->debug_checkOrdering(this->keys.back(), upper))
                        return false;
                } else {
                    if (!this->children[i]->debug_checkOrdering(this->keys[i - 1], this->keys[i]))
                        return false;
                }
            }
        }
        return true;
    }
    
    template <typename T>
    bool FineNode<T>::debug_checkChildCnt(int ordering) {
        if (this->isLeaf) {
            return numChild() == 0;
        }
        if (numKeys() <= 0) return false;
        if (numKeys() >= ordering) return false;
        if (numChild() != numKeys() + 1) return false;
        for (auto child : this->children) {
            bool childIsValid = child->debug_checkChildCnt(ordering);
            if (!childIsValid) return false;
        }
        return true;
    }

    template <typename T>
    void FineNode<T>::consolidateChild() {
        for (size_t id = 0; id < numChild(); id ++) {
            children[id]->parent = this;
            children[id]->childIndex = id;
        }
    }
}
