#ifndef ARM_SIMULATOR_H
#define ARM_SIMULATOR_H

#include "i_simulator.h"
#include "architecture.h"
#include "arm_to_ir.h"
#include "ir.h"
#include "memory.h"
#include "i_register_map.h"
#include <memory>
#include <map>
#include <string>
#include <vector>
#include "i_database_manager.h"
#include "arm_ui_manager.h"
#include "alu.h"
#include "fpu.h"
#include "vpu.h"

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
    void loadProgramFromString(const std::string& program_content);
    bool firstPass() override;
    bool secondPass() override;

    void accept(IRVisitor& visitor, const IRInstruction& instr) override;
    void execute_ir_instruction(const IRInstruction& ir_instr);


    // --- ISimulator Interface Implementation ---
    IRegisterMap& getRegisterMap() override { return *register_map_; }
    const IRegisterMap& getRegisterMap() const override { return *register_map_; }
    Memory& getMemory() override { return memory_; }
    const Memory& getMemory() const override { return memory_; }
    IDatabaseManager& getDatabaseManager() override { return db_manager_; }
    const Architecture& get_architecture() const override { return architecture_; }
    int get_session_id() const override { return session_id_; }

    const char* get_stack_pointer_name() const override { return "sp"; }
    const char* get_instruction_pointer_name() const override { return "pc"; }

    void set_ZF(bool value) override;
    void set_SF(bool value) override;
    void set_CF(bool value) override;
    void set_OF(bool value) override;
    void set_NZCV(bool n, bool z, bool c, bool v);

    bool get_ZF() const override;
    bool get_SF() const override;
    bool get_CF() const override;
    bool get_OF() const override;


    ProgramDecoder* getProgramDecoder() override;

    void set_PF(bool value) override;
    bool get_PF() const override;

    // System Register Access
    uint64_t get_system_register(const std::string& name) override;
    void set_system_register(const std::string& name, uint64_t value) override;

    ArmUIManager* getUIManager() { return ui_manager_.get(); }
    const std::map<std::string, address_t>& getSymbolTable() const { return symbolTable_; }

private:
    std::unique_ptr<ProgramDecoder> program_decoder_;
    IDatabaseManager& db_manager_;
    Memory& memory_;
    Architecture architecture_;
    std::unique_ptr<IRegisterMap> register_map_;
    IRProgram ir_program_;
    int session_id_;
    bool headless_;
    std::unique_ptr<ArmUIManager> ui_manager_;
    std::unique_ptr<ALU> alu_;
    std::unique_ptr<FPU> fpu_;
    std::unique_ptr<VPU> vpu_;
    std::map<std::string, address_t> symbolTable_;
    std::vector<std::string> programLines_;
    std::string entryPointLabel_ = "_start";
};

#endif // ARM_SIMULATOR_H
