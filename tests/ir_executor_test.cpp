#include "gtest/gtest.h"
#include "../x86_simulator.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include "../ir.h"
#include "../pipeline.h"
#include "../fetch_stage.h"

class IRExecutorTest : public ::testing::Test {
protected:
    MockDatabaseManager dbManager;
    Memory memory;
    std::unique_ptr<X86Simulator> simulator;

    IRExecutorTest() : memory(), simulator(std::make_unique<X86Simulator>(dbManager, memory, 1, true)) {}

    void SetUp() override {
        // Set up any necessary objects or state
    }

    void TearDown() override {
        // Clean up
    }

    void run_pipeline(int cycles) {
        for (int i = 0; i < cycles; ++i) {
            simulator->get_pipeline().cycle();
        }
    }
};

TEST_F(IRExecutorTest, HandleIrAnd) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 0b1100);
    regs.set32("ebx", 0b1010);

    IRInstruction and_instr(IROpcode::And, {
        std::string("eax"),
        std::string("ebx")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(and_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0b1000);
    EXPECT_FALSE(simulator->get_ZF());
    EXPECT_FALSE(simulator->get_SF());
    EXPECT_FALSE(simulator->get_CF());
    EXPECT_FALSE(simulator->get_OF());
}

TEST_F(IRExecutorTest, HandleIrOr) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 0b1100);
    regs.set32("ebx", 0b1010);

    IRInstruction or_instr(IROpcode::Or, {
        std::string("eax"),
        std::string("ebx")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(or_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0b1110);
    EXPECT_FALSE(simulator->get_ZF());
    EXPECT_FALSE(simulator->get_SF());
}

TEST_F(IRExecutorTest, HandleIrNot) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 0xFFFFFF00);

    IRInstruction not_instr(IROpcode::Not, {
        std::string("eax")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(not_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0x000000FF);
}

TEST_F(IRExecutorTest, HandleIrShl) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 0b1011);

    IRInstruction shl_instr(IROpcode::Shl, {
        std::string("eax"),
        (uint64_t)2
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(shl_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0b101100);
    EXPECT_FALSE(simulator->get_CF()); // Last bit shifted out was 0
}

TEST_F(IRExecutorTest, HandleIrShr) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 0b1011);

    IRInstruction shr_instr(IROpcode::Shr, {
        std::string("eax"),
        (uint64_t)2
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(shr_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0b10);
    EXPECT_TRUE(simulator->get_CF()); // Last bit shifted out was 1
}

TEST_F(IRExecutorTest, HandleIrSar) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 0b10000000000000000000000000001011); // Negative number

    IRInstruction sar_instr(IROpcode::Sar, {
        std::string("eax"),
        (uint64_t)2
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(sar_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0b11100000000000000000000000000010);
    EXPECT_TRUE(simulator->get_CF()); // Last bit shifted out was 1
}

TEST_F(IRExecutorTest, HandleIrInc) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 10);
    // Preserve initial CF
    simulator->set_CF(true);

    IRInstruction inc_instr(IROpcode::Inc, {
        std::string("eax")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(inc_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 11);
    EXPECT_FALSE(simulator->get_ZF());
    EXPECT_FALSE(simulator->get_SF());
    EXPECT_TRUE(simulator->get_CF()); // Check that CF is not affected
}

TEST_F(IRExecutorTest, HandleIrIncToZero) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", -1); // 0xFFFFFFFF

    IRInstruction inc_instr(IROpcode::Inc, {
        std::string("eax")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(inc_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0);
    EXPECT_TRUE(simulator->get_ZF());
    EXPECT_FALSE(simulator->get_SF());
}

TEST_F(IRExecutorTest, HandleIrDec) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 10);
    // Preserve initial CF
    simulator->set_CF(true);

    IRInstruction dec_instr(IROpcode::Dec, {
        std::string("eax")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(dec_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 9);
    EXPECT_FALSE(simulator->get_ZF());
    EXPECT_FALSE(simulator->get_SF());
    EXPECT_TRUE(simulator->get_CF()); // Check that CF is not affected
}

TEST_F(IRExecutorTest, HandleIrDecToZero) {
    auto& regs = simulator->getRegisterMap();
    regs.set32("eax", 1);

    IRInstruction dec_instr(IROpcode::Dec, {
        std::string("eax")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(dec_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("eax"), 0);
    EXPECT_TRUE(simulator->get_ZF());
    EXPECT_FALSE(simulator->get_SF());
}

TEST_F(IRExecutorTest, HandleIrPush) {
    auto& regs = simulator->getRegisterMap();
    address_t initial_rsp = memory.get_stack_bottom();
    regs.set64("rsp", initial_rsp);
    regs.set32("eax", 0xDEADBEEF);

    IRInstruction push_instr(IROpcode::Push, {
        std::string("eax")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(push_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get64("rsp"), initial_rsp - 4);
    EXPECT_EQ(memory.read_dword(regs.get64("rsp")), 0xDEADBEEF);
}

TEST_F(IRExecutorTest, HandleIrPop) {
    auto& regs = simulator->getRegisterMap();
    address_t initial_rsp = memory.get_stack_bottom() - 4;
    regs.set64("rsp", initial_rsp);
    memory.write_dword(initial_rsp, 0xCAFEBABE);

    IRInstruction pop_instr(IROpcode::Pop, {
        std::string("ecx")
    }, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(pop_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get32("ecx"), 0xCAFEBABE);
    EXPECT_EQ(regs.get64("rsp"), initial_rsp + 4);
}

TEST_F(IRExecutorTest, HandleIrRet) {
    auto& regs = simulator->getRegisterMap();
    address_t initial_rsp = memory.get_stack_bottom() - 8;
    uint64_t return_addr = 0x2000;
    regs.set64("rsp", initial_rsp);
    memory.write_qword(initial_rsp, return_addr);

    IRInstruction ret_instr(IROpcode::Ret, {}, FunctionalUnitType::ALU);

    simulator->get_pipeline().get_fetch_stage().inject_ir_instruction(ret_instr, simulator->get_pipeline());
    run_pipeline(5);

    EXPECT_EQ(regs.get64("rip"), return_addr);
    EXPECT_EQ(regs.get64("rsp"), initial_rsp + 8);
}
