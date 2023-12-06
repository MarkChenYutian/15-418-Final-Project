#include <vector>
#include "engine.hpp"
#include "fineTree/fineTree.hpp"

int main() {

    std::vector<std::string> Cases = {};
    for (int i = 0; i < 10; i ++) {
        std::string s = "../test/small_" + std::to_string(i) + ".case";
        Cases.push_back(s);
    }
    for (int i = 0; i < 10; i ++) {
        std::string s = "../test/large_" + std::to_string(i) + ".case";
        Cases.push_back(s);
    }
//    for (int i = 0; i < 3; i ++) {
//        std::string s = "../test/mega_" + std::to_string(i) + ".case";
//        Cases.push_back(s);
//    }
    {
        Engine::EngineConfig cfg {5, 1, Cases};
        auto runner = Engine::ThreadEngine<Tree::FineLockBPlusTree>(cfg);
        runner.Run();
    }


    return 0;
}
