#include "Builtins.h"

namespace autoflow {

// 各分类注册函数（分散在对应 .cpp 中）
void registerFlowInstructions();
void registerInputInstructions();
void registerDataInstructions();
void registerVisionInstructions();
void registerSystemInstructions();

void registerBuiltinInstructions() {
    static bool done = false;   // 幂等：防止重复注册导致指令面板出现重复项
    if (done) return;
    done = true;
    registerFlowInstructions();
    registerInputInstructions();
    registerDataInstructions();
    registerVisionInstructions();
    registerSystemInstructions();
}

} // namespace autoflow
