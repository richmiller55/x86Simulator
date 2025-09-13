#include "x86_simulator.h"
#include "decoder.h"

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

    DecodedInstruction decoded_instr = *decoded_instr_opt;
    // Log before execution
    // log(session_id_, "Executing: " + decoded_instr.mnemonic, "INFO", instruction_pointer, __FILE__, __LINE__);

    // EXECUTE
    bool success = executeInstruction(decoded_instr);

    if (success) {
        // ADVANCE THE INSTRUCTION POINTER
        // Jumps and other control flow instructions will have
        // already set the new instruction_pointer value within execute()
        // so we only update for non-jump instructions.
        if (decoded_instr.mnemonic != "JMP" && decoded_instr.mnemonic != "JNE" /* etc. */) {
            address_t new_ip = instruction_pointer + decoded_instr.length_in_bytes;
            register_map_.set64("rip", new_ip);
        } else {
            // JMP/JNE already updated rip
        }
    } else {
        log(session_id_, "Execution failed for: " + decoded_instr.mnemonic, "ERROR", instruction_pointer, __FILE__, __LINE__);
        // This is a good place to set a halt flag
        // isRunning_ = false;
    }
}

void X86Simulator::runProgram() {
    bool isRunning = true; // Use a flag for robust loop control
    
    while (isRunning) {
        address_t instruction_pointer = register_map_.get64("rip");
        
        // Check for end of program or explicit halt.
        if (instruction_pointer >= memory_.text_segment_start + memory_.text_segment_size) {
            isRunning = false; // Program finished
            log(session_id_, "End of program", "INFO", instruction_pointer, __FILE__, __LINE__);
        } else {
            runSingleInstruction();
        }

        // Add optional delays, UI updates, or user input handling here.
        // This is better than placing it inside the core execute function.

        ui_.waitForInput(); // Consider moving this to a specific "step-by-step" mode.
        
        // Optional delay for smoother viewing
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
