#include "gtest/gtest.h"
#include "../x86_simulator.h"
#include "../x86_to_ir.h"
#include "../ir.h"
#include "../memory.h"
#include "mock_database_manager.h"
#include "../program_decoder.h"

// Test fixture for IR translation and execution tests
class IRTranslationTest : public ::testing::Test {
protected:
    void SetUp() override {
        simulator = std::make_unique<X86Simulator>(mock_db_manager, memory, 0, true);
    }

    void run_pipeline(int cycles) {
        for (int i = 0; i < cycles; ++i) {
            simulator->get_pipeline().cycle();
        }
    }

    Memory memory;
    MockDatabaseManager mock_db_manager;
    std::unique_ptr<X86Simulator> simulator;
};

TEST_F(IRTranslationTest, MovImmediateToRegister) {
    // 1. Create a DecodedInstruction for "mov eax, 123"
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "mov";
    decoded_instr.address = 0x4000; // Dummy address
    decoded_instr.length_in_bytes = 5; // Typical length for mov eax, imm32
    
    DecodedOperand dest_op;
    dest_op.type = OperandType::REGISTER;
    dest_op.text = "eax";
    decoded_instr.operands.push_back(dest_op);

    DecodedOperand src_op;
    src_op.type = OperandType::IMMEDIATE;
    src_op.value = 123;
    src_op.text = "123";
    decoded_instr.operands.push_back(src_op);

    auto program_decoder = std::make_unique<ProgramDecoder>(simulator->getMemory());
    program_decoder->add_instruction(std::make_shared<DecodedInstruction>(decoded_instr));
    simulator->set_program_decoder(std::move(program_decoder));

    // 3. Set rip to the start of the program
    simulator->getRegisterMap().set64("rip", 0x4000);

    // 4. Run the pipeline for enough cycles to complete the instruction
    run_pipeline(5); // 5 stages in the pipeline

    // 5. Verify the result
    EXPECT_EQ(simulator->getRegisterMap().get32("eax"), 123);
}

TEST_F(IRTranslationTest, AddImmediateToRegister) {
    // 1. Set initial state
    simulator->getRegisterMap().set32("eax", 100);

    // 2. Create a DecodedInstruction for "add eax, 50"
    DecodedInstruction decoded_instr;
    decoded_instr.mnemonic = "add";
    decoded_instr.address = 0x4000;
    decoded_instr.length_in_bytes = 3;
    
    DecodedOperand dest_op;
    dest_op.type = OperandType::REGISTER;
    dest_op.text = "eax";
    decoded_instr.operands.push_back(dest_op);

    DecodedOperand src_op;
    src_op.type = OperandType::IMMEDIATE;
    src_op.value = 50;
    src_op.text = "50";
    decoded_instr.operands.push_back(src_op);

    // 3. Create a ProgramDecoder and add the instruction to it
    auto program_decoder = std::make_unique<ProgramDecoder>(*simulator);
    program_decoder->add_instruction(std::make_shared<DecodedInstruction>(decoded_instr));
    simulator->set_program_decoder(std::move(program_decoder));

    // 4. Set rip to the start of the program
    simulator->getRegisterMap().set64("rip", 0x4000);

    // 5. Run the pipeline
    run_pipeline(5);

    // 6. Verify the result
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
    decoded_instr.address = 0x4000;
    decoded_instr.length_in_bytes = 2;
    
    DecodedOperand dest_op;
    dest_op.type = OperandType::REGISTER;
    dest_op.text = "eax";
    decoded_instr.operands.push_back(dest_op);

    DecodedOperand src_op;
    src_op.type = OperandType::REGISTER;
    src_op.text = "ecx";
    decoded_instr.operands.push_back(src_op);

    // 3. Create a ProgramDecoder and add the instruction to it
    auto program_decoder = std::make_unique<ProgramDecoder>(*simulator);
    program_decoder->add_instruction(std::make_shared<DecodedInstruction>(decoded_instr));
    simulator->set_program_decoder(std::move(program_decoder));

    // 4. Set rip to the start of the program
    simulator->getRegisterMap().set64("rip", 0x4000);

    // 5. Run the pipeline
    run_pipeline(5);

    // 6. Verify the result
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
    cmp_instr.address = 0x4000;
    cmp_instr.length_in_bytes = 3;
    DecodedOperand cmp_op1;
    cmp_op1.type = OperandType::REGISTER;
    cmp_op1.text = "eax";
    cmp_instr.operands.push_back(cmp_op1);
    DecodedOperand cmp_op2;
    cmp_op2.type = OperandType::IMMEDIATE;
    cmp_op2.text = "100";
    cmp_op2.value = 100;
    cmp_instr.operands.push_back(cmp_op2);

    // 3. Create and execute "jne target"
    DecodedInstruction jne_instr;
    jne_instr.mnemonic = "jne";
    jne_instr.address = 0x4003;
    jne_instr.length_in_bytes = 2;
    DecodedOperand jne_op;
    jne_op.type = OperandType::IMMEDIATE;
    jne_op.text = "0x1000";
    jne_op.value = 0x1000;
    jne_instr.operands.push_back(jne_op);

    // 4. Create a ProgramDecoder and add the instructions to it
    auto program_decoder = std::make_unique<ProgramDecoder>(*simulator);
    program_decoder->add_instruction(std::make_shared<DecodedInstruction>(cmp_instr));
    program_decoder->add_instruction(std::make_shared<DecodedInstruction>(jne_instr));
    simulator->set_program_decoder(std::move(program_decoder));

    // 5. Set rip to the start of the program
    simulator->getRegisterMap().set64("rip", 0x4000);

    // 6. Run the pipeline
    run_pipeline(6); // 2 instructions, 5 stages + 1 cycle

    // 7. Verify flags after CMP
    EXPECT_TRUE(simulator->get_ZF()); // Zero flag should be true

    // 8. Verify that RIP has NOT changed by the JNE
    EXPECT_EQ(simulator->getRegisterMap().get64("rip"), 0x4005);
}