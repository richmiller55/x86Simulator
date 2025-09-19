#include "x86_simulator.h"
#include "decoder.h"
#include "ui_manager.h"

bool X86Simulator::executeInstruction(const DecodedInstruction& decoded_instr) {
    // Convert instruction mnemonic to normalized format
    std::string normalized_mnemonic = decoded_instr.mnemonic;
    std::transform(normalized_mnemonic.begin(), normalized_mnemonic.end(),
                   normalized_mnemonic.begin(), ::toupper);
    // Move waitForInput() out of here. It doesn't belong inside the core execution logic.
    // ui_.waitForInput();

    // The logic inside this function looks good.
    if (normalized_mnemonic == "MOV") {
        if (decoded_instr.operands.size() == 2) {
            handleMov(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "ADD") {
        if (decoded_instr.operands.size() == 2) {
            handleAdd(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "SUB") {
        if (decoded_instr.operands.size() == 2) {
            handleSub(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "JMP") {
        if (!decoded_instr.operands.empty()) {
            handleJmp(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "INC") {
        if (!decoded_instr.operands.empty()) {
            handleInc(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "CMP") {
        if (decoded_instr.operands.size() == 2) {
            handleCmp(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "JNE") {
        if (!decoded_instr.operands.empty()) {
            handleJne(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "INT") {
        if (!decoded_instr.operands.empty()) {
            handleInt(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "MUL") {
        if (!decoded_instr.operands.empty()) {
            handleMul(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "DEC") {
        if (!decoded_instr.operands.empty()) {
            handleDec(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "DIV") {
        if (!decoded_instr.operands.empty()) {
            handleDiv(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "AND") {
        if (decoded_instr.operands.size() == 2) {
            handleAnd(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "OR") {
        if (decoded_instr.operands.size() == 2) {
            handleOr(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "XOR") {
        if (decoded_instr.operands.size() == 2) {
            handleXor(decoded_instr);
            return true;
        }
    } else if (normalized_mnemonic == "NOT") {
        if (decoded_instr.operands.size() == 1) {
            handleNot(decoded_instr);
            return true;
        }
    } else {
        std::string logmessage = "unsupported instruction: " + decoded_instr.mnemonic;
        log(session_id_, logmessage, "ERROR", instructionPointer_, __FILE__, __LINE__);
        return false;
    }
    return false;
}

void X86Simulator::runSingleInstruction() {
    address_t instruction_pointer = register_map_.get64("rip");

    // FETCH & DECODE
    Decoder& decoder = Decoder::getInstance();
    auto decoded_instr_opt = decoder.decodeInstruction(memory_, instruction_pointer);

    if (!decoded_instr_opt) {
        log(session_id_, "Decoding failed at RIP: " + std::to_string(instruction_pointer), "ERROR", instruction_pointer, __FILE__, __LINE__);
        // This is a good place to set a halt flag
        // isRunning_ = false; 
        return;
    }

    auto decoded_instr = std::move(decoded_instr_opt);
    // Log before execution
    // log(session_id_, "Executing: " + decoded_instr.mnemonic, "INFO", instruction_pointer, __FILE__, __LINE__);

    // EXECUTE
    address_t next_ip = instruction_pointer + decoded_instr->length_in_bytes;
    bool success = executeInstruction(*decoded_instr);

    if (success) {
        // If the instruction pointer was not modified by a jump, advance it.
        // Jumps and conditional jumps will set RIP themselves.
        // We check if RIP is still pointing to the current instruction.
        if (register_map_.get64("rip") == instruction_pointer) {
            register_map_.set64("rip", next_ip);
        }
        // Otherwise, a jump occurred and RIP is already correct.
    } else {
        log(session_id_, "Execution failed for: " + decoded_instr->mnemonic, "ERROR", instruction_pointer, __FILE__, __LINE__);
        // This is a good place to set a halt flag
        // isRunning_ = false;
    }
}

void X86Simulator::runProgram() {
    bool isRunning = true; // Use a flag for robust loop control
    
    while (isRunning) {
        updateDisplay(); // Update the UI with the new state (before waiting for input)

        if (!ui_->waitForInput()) { // Wait for user to press a key
            isRunning = false;
            continue;
        }

        address_t instruction_pointer = register_map_.get64("rip");
        
        // Check for end of program or explicit halt.
        if (instruction_pointer >= memory_.text_segment_start + memory_.text_segment_size) {
            isRunning = false; // Program finished
            log(session_id_, "End of program", "INFO", instruction_pointer, __FILE__, __LINE__);
        } else {
            runSingleInstruction();
        }
    }
}

void X86Simulator::dumpTextSegment(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    Decoder& decoder = Decoder::getInstance();
    address_t current_address = memory_.text_segment_start;

    while (current_address < memory_.data_segment_start && current_address < memory_.text_segment_start + memory_.text_segment_size) {
        auto decoded_instr_opt = decoder.decodeInstruction(memory_, current_address);
        if (!decoded_instr_opt) {
            // If decoding fails, just print the byte and move on
            outfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << current_address << ": "
                    << std::setw(2) << (int)memory_.read_text(current_address) << "   (decode failed)" << std::endl;
            current_address++;
            continue;
        }

        auto decoded_instr = std::move(decoded_instr_opt);

        // Print Address
        outfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << decoded_instr->address << ": ";

        // Print Raw Bytes
        std::stringstream bytes_ss;
        for (size_t i = 0; i < decoded_instr->length_in_bytes; ++i) {
            bytes_ss << std::hex << std::setw(2) << std::setfill('0') << (int)memory_.read_text(decoded_instr->address + i) << " ";
        }
        outfile << std::left << std::setw(18) << bytes_ss.str();

        // Print Disassembled Instruction
        outfile << " " << decoded_instr->mnemonic;
        for (const auto& op : decoded_instr->operands) {
            outfile << " " << op.text;
        }
        outfile << std::endl;

        current_address += decoded_instr->length_in_bytes;
    }
    outfile.close();
}
