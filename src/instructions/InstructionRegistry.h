#pragma once
#include "IInstruction.h"

namespace autoflow {

// 指令注册表（插件化：新增指令不改核心引擎）
class InstructionRegistry {
public:
    static InstructionRegistry& instance();

    void add(std::unique_ptr<IInstruction> instr);

    IInstruction* get(const std::string& id);
    const std::vector<IInstruction*>& all() const { return m_all; }
    std::vector<IInstruction*> byCategory(const std::string& cat) const;
    std::vector<std::string> categories() const;

private:
    InstructionRegistry() = default;
    std::vector<std::unique_ptr<IInstruction>> m_owned;
    std::vector<IInstruction*> m_all;
    std::map<std::string, IInstruction*> m_byId;
};

// 便捷注册助手
inline void registerInstruction(std::unique_ptr<IInstruction> i) {
    InstructionRegistry::instance().add(std::move(i));
}

} // namespace autoflow
