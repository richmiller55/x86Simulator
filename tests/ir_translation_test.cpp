
#include "gtest/gtest.h"
#include "../x86_simulator.h"
#include "../x86_to_ir.h"
#include "../ir.h"
#include "../memory.h"
#include "mock_database_manager.h"

// Test fixture for IR translation and execution tests
class IRTranslationTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_manager = &MockDatabaseManager::getInstance();
        simulator = std::make_unique<X86Simulator>(*db_manager, memory, 0, true);
    }

    Memory memory;
    IDatabaseManager* db_manager;
    std::unique_ptr<X86Simulator> simulator;
};

TEST_F(IRTranslationTest, MovImmediateToRegister) {
    // 1. Create a DecodedInstruction for "mov eax, 123"
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "mov";
    
    DecodedOperand dest_op;
    dest_op.type = OperandType::REGISTER;
    dest_op.text = "eax";
    decoded_instr.operands.push_back(dest_op);

    DecodedOperand src_op;
    src_op.type = OperandType::IMMEDIATE;
    src_op.value = 123;
    src_op.text = "123";
    decoded_instr.operands.push_back(src_op);

    // 2. Translate to IR
    auto ir_instr = translate_to_ir(decoded_instr);
    ASSERT_NE(ir_instr, nullptr);
    ASSERT_EQ(ir_instr->opcode, IROpcode::Move);

    // 3. Execute the IR instruction
    simulator->execute_ir_instruction(*ir_instr);

    // 4. Verify the result
    EXPECT_EQ(simulator->getRegisterMap().get32("eax"), 123);
}

TEST_F(IRTranslationTest, AddImmediateToRegister) {
    // 1. Set initial state
    simulator->getRegisterMap().set32("eax", 100);

    // 2. Create a DecodedInstruction for "add eax, 50"
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "add";
    
    DecodedOperand dest_op;
    dest_op.type = OperandType::REGISTER;
    dest_op.text = "eax";
    decoded_instr.operands.push_back(dest_op);

    DecodedOperand src_op;
    src_op.type = OperandType::IMMEDIATE;
    src_op.value = 50;
    src_op.text = "50";
    decoded_instr.operands.push_back(src_op);

    // 3. Translate to IR
    auto ir_instr = translate_to_ir(decoded_instr);
    ASSERT_NE(ir_instr, nullptr);
    ASSERT_EQ(ir_instr->opcode, IROpcode::Add);

    // 4. Execute the IR instruction
    simulator->execute_ir_instruction(*ir_instr);

    // 5. Verify the result
    EXPECT_EQ(simulator->getRegisterMap().get32("eax"), 150);
    EXPECT_FALSE(simulator->get_ZF()); // Zero flag should be false
}

TEST_F(IRTranslationTest, SubRegisterFromRegister) {
    // 1. Set initial state
    simulator->getRegisterMap().set32("eax", 100);
    simulator->getRegisterMap().set32("ecx", 30);

    // 2. Create a DecodedInstruction for "sub eax, ecx"
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "sub";
    
    DecodedOperand dest_op;
    dest_op.type = OperandType::REGISTER;
    dest_op.text = "eax";
    decoded_instr.operands.push_back(dest_op);

    DecodedOperand src_op;
    src_op.type = OperandType::REGISTER;
    src_op.text = "ecx";
    decoded_instr.operands.push_back(src_op);

    // 3. Translate to IR
    auto ir_instr = translate_to_ir(decoded_instr);
    ASSERT_NE(ir_instr, nullptr);
    ASSERT_EQ(ir_instr->opcode, IROpcode::Sub);

    // 4. Execute the IR instruction
    simulator->execute_ir_instruction(*ir_instr);

    // 5. Verify the result
    EXPECT_EQ(simulator->getRegisterMap().get32("eax"), 70);
    EXPECT_FALSE(simulator->get_ZF());
    EXPECT_FALSE(simulator->get_CF());
}

TEST_F(IRTranslationTest, CmpAndJump) {
    // Test the sequence: cmp eax, 100; jne target;
    
    // 1. Set initial state
    simulator->getRegisterMap().set32("eax", 100);

    // 2. Create and execute "cmp eax, 100"
    DecodedInstruction cmp_instr;
    cmp_instr.mnemonic = "cmp";
    cmp_instr.operands.push_back({OperandType::REGISTER, "eax"});
    cmp_instr.operands.push_back({OperandType::IMMEDIATE, "100", 100});

    auto ir_cmp = translate_to_ir(cmp_instr);
    ASSERT_NE(ir_cmp, nullptr);
    simulator->execute_ir_instruction(*ir_cmp);

    // 3. Verify flags after CMP
    EXPECT_TRUE(simulator->get_ZF()); // Zero flag should be true

    // 4. Create and execute "jne target" (we expect it NOT to jump)
    address_t original_rip = simulator->getRegisterMap().get64("rip");
    DecodedInstruction jne_instr;
    jne_instr.mnemonic = "jne";
    jne_instr.operands.push_back({OperandType::IMMEDIATE, "0x1000", 0x1000});

    auto ir_jne = translate_to_ir(jne_instr);
    ASSERT_NE(ir_jne, nullptr);
    simulator->execute_ir_instruction(*ir_jne);

    // 5. Verify that RIP has NOT changed
    EXPECT_EQ(simulator->getRegisterMap().get64("rip"), original_rip);
}
