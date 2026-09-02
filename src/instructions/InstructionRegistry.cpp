#include "InstructionRegistry.h"

namespace autoflow {

InstructionRegistry& InstructionRegistry::instance() {
    static InstructionRegistry inst;
    return inst;
}

void InstructionRegistry::add(std::unique_ptr<IInstruction> instr) {
    IInstruction* raw = instr.get();
    m_owned.push_back(std::move(instr));
    m_all.push_back(raw);
    m_byId[raw->meta().id] = raw;
}

IInstruction* InstructionRegistry::get(const std::string& id) {
    auto it = m_byId.find(id);
    return it == m_byId.end() ? nullptr : it->second;
}

std::vector<IInstruction*> InstructionRegistry::byCategory(const std::string& cat) const {
    std::vector<IInstruction*> out;
    for (auto* i : m_all) if (i->meta().category == cat) out.push_back(i);
    return out;
}

std::vector<std::string> InstructionRegistry::categories() const {
    std::vector<std::string> out;
    for (auto* i : m_all) {
        std::string c = i->meta().category;
        if (std::find(out.begin(), out.end(), c) == out.end()) out.push_back(c);
    }
    return out;
}

} // namespace autoflow
