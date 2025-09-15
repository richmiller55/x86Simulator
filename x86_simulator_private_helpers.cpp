#include "x86_simulator.h"
#include "decoder.h"

// In x86_simulator_private_helpers.cpp or x86_simulator.cpp (depending on where they are defined)

void X86Simulator::handleMov(const DecodedInstruction& decoded_instr) {
    // We expect exactly two operands for a MOV instruction
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for MOV", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    // The value of the source operand is now readily available
    const uint64_t sourceValue = src_operand.value;

    // Use the register_map_ to set the destination register's value
    try {
        register_map_.set64(dest_operand.text, sourceValue);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in MOV: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

// In x86_simulator_private_helpers.cpp or x86_simulator.cpp

void X86Simulator::handleAdd(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for ADD", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    uint64_t destValue = 0;
    // Get the destination register value using its text representation from register_map_
    try {
        destValue = register_map_.get64(dest_operand.text);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in ADD: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // The source value is available directly from the DecodedOperand
    const uint64_t sourceValue = src_operand.value;

    // Perform addition and update destination using register_map_
    try {
        register_map_.set64(dest_operand.text, destValue + sourceValue);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in ADD: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleSub(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for SUB", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& dest_operand = decoded_instr.operands[0];
    const DecodedOperand& src_operand = decoded_instr.operands[1];

    uint64_t destValue = 0;
    // Get the destination register value using its text representation from register_map_
    try {
        destValue = register_map_.get64(dest_operand.text);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in SUB: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    // The source value is available directly from the DecodedOperand
    const uint64_t sourceValue = src_operand.value;

    // Perform subtraction and update destination using register_map_
    try {
        register_map_.set64(dest_operand.text, destValue - sourceValue);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in SUB: " + dest_operand.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleJmp(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JMP instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    const std::string& targetLabel = target_operand.text; // The label name is in the 'text' field

    auto it = symbolTable_.find(targetLabel);

    if (it != symbolTable_.end()) {
        address_t targetAddress = it->second;
        if (targetAddress == 0) {
            std::string logMessage = "JMP target label '" + targetLabel + "' resolves to an invalid address.";
            log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        } else {
            // Jumps modify the instruction pointer directly
            register_map_.set64("rip", targetAddress);
        }
    } else {
        std::string logMessage = "JMP target must be a valid label or address. Label '" + targetLabel + "' not found.";
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleJne(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "JNE instruction requires a target.", "ERROR",
	    instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& target_operand = decoded_instr.operands[0];
    if (get_ZF() == false) { // If Zero Flag is not set, then jump
        const std::string& targetLabel = target_operand.text;
        auto it = symbolTable_.find(targetLabel);
        if (it != symbolTable_.end()) {
            address_t targetAddress = it->second;
            register_map_.set64("rip", targetAddress);
        } else {
            std::string logMessage = "JNE target must be a valid label. Label '" + targetLabel + "' not found.";
            log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        }
    }
    // If ZF is set, do nothing and let the instruction pointer advance normally.
}

void X86Simulator::handleInc(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.empty()) {
        log(session_id_, "INC instruction requires an operand.", "ERROR",
            instructionPointer_, __FILE__, __LINE__);
        return;
    }

    const DecodedOperand& operand = decoded_instr.operands[0];
    if (operand.type == OperandType::REGISTER) {
        try {
            uint64_t value = getRegister(operand.text);
            value++;
            register_map_.set64(operand.text, value);
            // TODO: Update RFLAGS (OF, SF, ZF, AF, PF)
        } catch (const std::out_of_range& e) {
            std::string logMessage = "Invalid register in INC: " + operand.text;
            log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        }
    } else {
        log(session_id_, "INC only supports register operands currently.", "ERROR", instructionPointer_, __FILE__, __LINE__);
    }
}

void X86Simulator::handleCmp(const DecodedInstruction& decoded_instr) {
    if (decoded_instr.operands.size() != 2) {
        log(session_id_, "Invalid number of operands for CMP.", "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }
    
    const DecodedOperand& operand1 = decoded_instr.operands[0];
    const DecodedOperand& operand2 = decoded_instr.operands[1];

    uint64_t val1 = 0;
    try {
        val1 = getRegister(operand1.text);
    } catch (const std::out_of_range& e) {
        std::string logMessage = "Invalid destination operand in CMP: " + operand1.text;
        log(session_id_, logMessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return;
    }

    uint64_t val2 = operand2.value; // Assuming immediate for now
    uint64_t result = val1 - val2;

    set_ZF(result == 0);
    set_SF((result & 0x8000000000000000) != 0);
    // TODO: Set CF and OF correctly for subtraction.
}