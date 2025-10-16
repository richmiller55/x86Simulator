#ifndef ARM_SIMULATOR_H
#define ARM_SIMULATOR_H

#include "i_simulator.h"
#include "architecture.h"
#include "arm_to_ir.h"
#include "ir.h"
#include "memory.h"
#include "register_map.h"
#include "i_database_manager.h"

/**
 * @class ArmSimulator
 * @brief An implementation of ISimulator for the ARM architecture.
 */
class ArmSimulator : public ISimulator {
public:
    ArmSimulator(IDatabaseManager& db_manager, Memory& memory, int session_id, bool headless);
    ~ArmSimulator();

    void runProgram() override;
    bool loadProgram(const std::string& program_path) override;

    // --- ISimulator Interface Implementation ---
    RegisterMap& getRegisterMap() override { return register_map_; }
    const RegisterMap& getRegisterMap() const override { return register_map_; }
    Memory& getMemory() override { return memory_; }
    const Memory& getMemory() const override { return memory_; }
    IDatabaseManager& getDatabaseManager() override { return db_manager_; }
    const Architecture& get_architecture() const override { return architecture_; }
    int get_session_id() const override { return session_id_; }

    void set_ZF(bool value) override;
    void set_SF(bool value) override;
    void set_CF(bool value) override;
    void set_OF(bool value) override;

    bool get_ZF() const override;
    bool get_SF() const override;
    bool get_CF() const override;
    bool get_OF() const override;

    void accept(IRVisitor& visitor, const IRInstruction& instr) override;

    void execute_ir_instruction(const IRInstruction& ir_instr) override;
    ProgramDecoder* getProgramDecoder() override;

    void set_PF(bool value) override;
    bool get_PF() const override;

private:
    std::unique_ptr<ProgramDecoder> program_decoder_;
    IDatabaseManager& db_manager_;
    Memory& memory_;
    RegisterMap register_map_;
    Architecture architecture_;
    IRProgram ir_program_;
    int session_id_;
    bool headless_;
    uint32_t cpsr_; // To hold the state of the Current Program Status Register
};

#endif // ARM_SIMULATOR_H
