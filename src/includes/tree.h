#pragma once

#include <iostream>
#include <algorithm>
#include <deque>
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include <memory>
#include <optional>
#include <cassert>

#ifdef DEBUG
std::mutex print_mutex;
#define DBG_PRINT(arg) \
    do { \
        std::lock_guard<std::mutex> lock(print_mutex); \
        arg; \
    } while (0);
#else
#define DBG_PRINT(arg) {}
#endif



namespace Tree {
    template <typename T>
    class ITree {
        public:
            bool debug_checkIsValid(bool verbose);
            int  size();
            
            void insert(T key);
            bool remove(T key);
            void print();
            std::optional<T> get(T key);
            std::vector<T> toVec();
    };

    template <typename T>
    /**
     * NOTE: A tree node for sequential version of B+ tree
     */
    struct SeqNode {
        bool isLeaf;                       // Check if node is leaf node
        bool isDummy;                      // Check if node is dummy node
        int  childIndex;                   // Which child am I in parent? (-1 if no parent)
        T    minElem;                      // Min element in the subtree rooted at this node (include this node itself)
        std::deque<T> keys;                // Keys
        std::deque<SeqNode<T>*> children;  // Children
        SeqNode<T>* parent;                // Pointer to parent node
        SeqNode<T>* next;                  // Pointer to left sibling
        SeqNode<T>* prev;                  // Pointer to right sibling

        SeqNode(bool leaf, bool dummy=false) : isLeaf(leaf), isDummy(dummy), parent(nullptr), next(nullptr), prev(nullptr), childIndex(-1) {};
        void printKeys();
        void releaseAll();
        void consolidateChild();
        bool debug_checkParentPointers();
        bool debug_checkOrdering(std::optional<T> lower, std::optional<T> upper);
        bool debug_checkChildCnt(int ordering, bool allowEmpty=false);
        void updateMin() {
            if (isLeaf && numKeys() > 0) {
                minElem = keys.front();
            } else if (!isLeaf) {
                assert (numChild() > 0);
                minElem = children[0]->minElem;
            }
        }

        inline size_t numKeys()  {return keys.size();}
        inline size_t numChild() {return children.size();}
        /**
         * Return the index of first key that is greater than or equal to "key".
         * 
         * NOTE:
         * If the key is larger than all keys in node, will return an
         * **out-of-bound** index!
         */
        inline size_t getGtKeyIdx(T key) {
            size_t index = 0;
            while (index < numKeys() && keys[index] <= key) index ++;
            return index;
        }
    };

    /**
     * NOTE: A tree node for finegrained locked version of B+ tree
     */
    template <typename T>
    struct LockNode {
        std::shared_mutex latch;

        bool isLeaf;                        // Check if node is leaf node
        bool isDummy;                       // Check if node is dummy node
        int childIndex;                     // Which child am I in parent? (-1 if no parent)
        std::deque<T> keys;                 // Keys
        std::deque<LockNode<T>*> children;  // Children
        LockNode<T>* parent;                // Pointer to parent node
        LockNode<T>* next;                  // Pointer to left sibling
        LockNode<T>* prev;                  // Pointer to right sibling

        LockNode(bool leaf, bool dummy=false) : isLeaf(leaf), isDummy(dummy), parent(nullptr), next(nullptr), prev(nullptr) {};
        /**
         * TODO: Maybe need to grab the unique lock on destruct?
         */
        // ~LockNode() {std::unique_lock lock(read_latch);}

        /**
         * Regenerate the node's keys based on current child.
         * NOTE: SIDE_EFFECT - will delete empty children automatically!
         */
        void rebuild();
        void printKeys();
        void releaseAll();
        void consolidateChild();
        bool debug_checkParentPointers();
        bool debug_checkOrdering(std::optional<T> lower, std::optional<T> upper);
        bool debug_checkChildCnt(int ordering);

        inline size_t numKeys()  {return keys.size();}
        inline size_t numChild() {return children.size();}
        inline size_t getGtKeyIdx(T key) {
            size_t index = 0;
            while (index < numKeys() && keys[index] <= key) index ++;
            return index;
        }
    };

    /**
     * NOTE: A datastructure used to keep track of the locks retrieved by
     * a single thread.
     * **Only used as a private variable within each thread, never share to others!**
     */
    template <typename T>
    struct LockDeque {
        bool isShared;
        std::deque<LockNode<T>*> nodes;
        
        LockDeque(bool isShared = false): isShared(isShared){}
        void retrieveLock(LockNode<T> *ptr) {
            if (isShared) ptr->latch.lock_shared();
            else ptr->latch.lock();
            nodes.push_back(ptr);
        }
        bool isLocked(LockNode<T> *ptr) {
            for (const auto node : nodes) {
                if (node == ptr) return true;
            }
            return false;
        }
        void releaseAllWriteLocks() {
            assert (!isShared);
            while (!nodes.empty()) {
                nodes.front()->latch.unlock();
                nodes.pop_front();
            }
        }
        void releasePrevWriteLocks() {
            assert (!isShared);
            while (nodes.size() > 1) {
                nodes.front()->latch.unlock();
                nodes.pop_front();
            }
        }
        void releaseAllReadLocks() {
            assert (isShared);
            while (!nodes.empty()) {
                nodes.front()->latch.unlock_shared();
                nodes.pop_front();
            }
        }
        void releasePrevReadLocks() {
            assert (isShared);
            while (nodes.size() > 1) {
                nodes.front()->latch.unlock_shared();
                nodes.pop_front();
            }
        }
        void popAndDelete(LockNode<T> *ptr) {
            assert(!isShared);
            assert(isLocked(ptr->parent));
            for (size_t idx = 0; idx < nodes.size(); idx ++) {
                if (nodes[idx] == ptr) {
                    nodes.erase(nodes.begin() + idx);
                    break;
                }
            }
            delete ptr;
        }
    };

    template<typename T>
    class SeqBPlusTree : public ITree<T> {
        private:
            SeqNode<T> rootPtr;
            int ORDER_;
            int size_;

        public:
            SeqBPlusTree(int order = 3);
            ~SeqBPlusTree();

            // Getter
            SeqNode<T>* getRoot();
            bool debug_checkIsValid(bool verbose);
            int  size();

            // Public Tree API
            void insert(T key);
            bool remove(T key);
            void print();
            std::optional<T> get(T key);
            std::vector<T> toVec();

        private:
            // Private helper functions
            SeqNode<T>* findLeafNode(SeqNode<T>* node, T key);
            void splitNode(SeqNode<T>* node, T key);
            void insertKey(SeqNode<T>* node, T key);
            bool removeFromLeaf(SeqNode<T>* node, T key);

            bool isHalfFull(SeqNode<T>* node);
            bool moreHalfFull(SeqNode<T>* node);

            void removeBorrow(SeqNode<T>* node);
            void removeMerge(SeqNode<T>* node);
    };

    template<typename T>
    class CoarseLockBPlusTree : public ITree<T> {
        private:
            std::mutex lock;
            SeqBPlusTree<T> tree;
        public:
            CoarseLockBPlusTree(int order = 3);
            ~CoarseLockBPlusTree();
            bool debug_checkIsValid(bool verbose);
            int  size();
            
            void insert(T key);
            bool remove(T key);
            void print();
            std::optional<T> get(T key);
            std::vector<T> toVec();
    };

    template<typename T>
    class FineLockBPlusTree : public ITree<T> {
        private:
            LockNode<T> rootPtr;
            int ORDER_;
            std::atomic<int> size_ = 0;
            std::shared_mutex rootLock;
        
        public:
            FineLockBPlusTree(int order = 3);
            ~FineLockBPlusTree();
            bool debug_checkIsValid(bool verbose);
            int  size();
            
            void insert(T key);
            bool remove(T key);
            void print();
            std::optional<T> get(T key);
            std::vector<T> toVec();
            LockNode<T> *getRoot();
        
        private:
            // Private helper functions
            LockNode<T>* findLeafNodeInsert(LockNode<T>* node, T key, LockDeque<T> &dq);
            LockNode<T>* findLeafNodeDelete(LockNode<T>* node, T key, LockDeque<T> &dq);
            LockNode<T>* findLeafNodeRead(LockNode<T>* node, T key, LockDeque<T> &dq);

            void splitNode(LockNode<T>* node, T key);
            void insertKey(LockNode<T>* node, T key);
            bool removeFromLeaf(LockNode<T>* node, T key);

            bool isHalfFull(LockNode<T>* node);
            bool moreHalfFull(LockNode<T>* node);

            void removeBorrow(LockNode<T>* node, LockDeque<T> &dq);
            void removeMerge(LockNode<T>* node, LockDeque<T> &dq);
    };
}
