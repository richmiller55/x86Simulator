#include "x86_simulator.h"
#include "decoder.h"
#include "ui_manager.h"
#include "x86_to_ir.h"
#include "INTEL_helpers.h"

#include "ir_executor_helpers.h"
#include <string>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>

void X86Simulator::execute_ir_instruction(const IRInstruction& ir_instr) {
    switch (ir_instr.opcode) {
        case IROpcode::Add:
            handle_ir_add(ir_instr, *this);
            break;
        case IROpcode::Sub:
            handle_ir_sub(ir_instr, *this);
            break;
        case IROpcode::Move:
            handle_ir_move(ir_instr, *this);
            break;
        case IROpcode::Load:
            handle_ir_load(ir_instr, *this);
            break;
        case IROpcode::Store:
            handle_ir_store(ir_instr, *this);
            break;
        case IROpcode::PackedAddPS:
            handle_ir_packed_add_ps(ir_instr, *this);
            break;
        default:
            db_manager_.log(session_id_, "Unhandled IR opcode", "ERROR", 0, __FILE__, __LINE__);
            break;
    }
}

// Rewritten executeInstruction to use the new IR pipeline
bool X86Simulator::executeInstruction(const DecodedInstruction& decoded_instr) {
    // 1. Translate the decoded x86 instruction to our abstract IR
    auto ir_instr = translate_to_ir(decoded_instr);

    // 2. Check if the translation was successful
    if (ir_instr) {
        // 3. Execute the IR instruction
        execute_ir_instruction(*ir_instr);
        return true;
    } else {
        // If translation is not supported yet, log an error.
        std::string logmessage = "Unsupported instruction for IR translation: " + decoded_instr.mnemonic;
        db_manager_.log(session_id_, logmessage, "ERROR", register_map_->get64("rip"), __FILE__, __LINE__);
        return false;
    }
}



void X86Simulator::runSingleInstruction() {
    address_t instruction_pointer = register_map_->get64("rip");

    // FETCH & DECODE
    Decoder& decoder = Decoder::getInstance();
    auto decoded_instr_opt = decoder.decodeInstruction(memory_, instruction_pointer);

    if (!decoded_instr_opt) {
        db_manager_.log(session_id_, "Decoding failed at RIP: " + std::to_string(instruction_pointer), "ERROR", instruction_pointer, __FILE__, __LINE__);
        return;
    }

    auto decoded_instr = std::move(decoded_instr_opt);

    if (decoded_instr->length_in_bytes == 0) {
        db_manager_.log(session_id_, "Decoder returned 0-length instruction at address " + std::to_string(instruction_pointer), "ERROR", instruction_pointer, __FILE__, __LINE__);
        register_map_->set64("rip", instruction_pointer + 1); // Prevent infinite loop
        return;
    }

    // EXECUTE
    address_t next_ip = instruction_pointer + decoded_instr->length_in_bytes;
    bool success = executeInstruction(*decoded_instr);

    if (success) {
        if (register_map_->get64("rip") == instruction_pointer) {
            register_map_->set64("rip", next_ip);
	    }
    } else {
        db_manager_.log(session_id_, "Execution failed for: " + decoded_instr->mnemonic, "ERROR", instruction_pointer, __FILE__, __LINE__);
    }

    update_rflags_in_register_map();
}

void X86Simulator::runProgram() {
    if (headless_) { // Handle headless mode separately
        while (true) {
            address_t instruction_pointer = register_map_->get64("rip");
            if (instruction_pointer >= memory_.get_text_segment_start() + memory_.get_text_segment_size()) {
                db_manager_.log(session_id_, "End of program", "INFO", instruction_pointer, __FILE__, __LINE__);
                break; // Program finished
            }
            pipeline_->cycle();
        }
        return;
    }

    // --- UI Mode ---
    bool isRunning = true;

    // Initial Draw
    ui_->drawMainRegisters(*register_map_);
    ui_->drawYmmRegisters(*register_map_);
    ui_->drawTextWindow(register_map_->get64("rip"));
    ui_->drawInstructionDescription(register_map_->get64("rip"), *register_map_);
    ui_->drawPipelineWindow();
    ui_->drawLegend();

    while (isRunning) {
        ui_->refreshAll();

        if (!ui_->waitForInput()) { // Blocks for 'n' or 'q'
            isRunning = false;
            continue;
        }

        // User pressed 'n', so execute one cycle
        pipeline_->cycle();

        // Check for end of program
        address_t instruction_pointer = register_map_->get64("rip");
        if (instruction_pointer >= memory_.get_text_segment_start() + memory_.get_text_segment_size()) {
            isRunning = false;
            db_manager_.log(session_id_, "End of program", "INFO", instruction_pointer, __FILE__, __LINE__);
        }

        // Update UI with new state
        ui_->drawMainRegisters(*register_map_);
        ui_->drawYmmRegisters(*register_map_);
        ui_->drawTextWindow(register_map_->get64("rip"));
        ui_->drawInstructionDescription(register_map_->get64("rip"), *register_map_);
        ui_->drawPipelineWindow();
    }
}

void X86Simulator::dumpTextSegment(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    Decoder& decoder = Decoder::getInstance();
    address_t current_address = memory_.get_text_segment_start();

    while (current_address < memory_.get_text_segment_start() + program_size_in_bytes_) {
        auto decoded_instr_opt = decoder.decodeInstruction(memory_, current_address);
        if (!decoded_instr_opt) {
            outfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << current_address << ": "
                    << std::setw(2) << (int)memory_.read_text(current_address) << "   (decode failed)" << std::endl;
            current_address++;
            continue;
        }

        auto decoded_instr = std::move(decoded_instr_opt);

        outfile << "0x" << std::hex << std::setw(8) << std::right << std::setfill('0') << decoded_instr->address << ": ";

        std::stringstream bytes_ss;
        for (size_t i = 0; i < decoded_instr->length_in_bytes; ++i) {
	        bytes_ss << std::hex << std::setw(2) << std::setfill('0') << (int)memory_.read_text(decoded_instr->address + i) << " ";
        }
        std::string bytes_str = bytes_ss.str();
        outfile << bytes_str;
        for (size_t i = bytes_str.length(); i < 24; ++i) {
            outfile << ' ';
        }

        outfile << " " << decoded_instr->mnemonic;
        for (const auto& op : decoded_instr->operands) {
            outfile << " " << op.text;
        }
        outfile << std::endl;

        if (decoded_instr->length_in_bytes == 0) {
            db_manager_.log(session_id_, "Decoder returned 0-length instruction at address " + std::to_string(current_address), "ERROR", current_address, __FILE__, __LINE__);
            current_address++;
        }
        else {
            current_address += decoded_instr->length_in_bytes;
        }
    }
    outfile.close();
}

void X86Simulator::dumpMemoryRange(const std::string& filename, address_t start_addr, size_t size) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    const int bytes_per_line = 16;
    for (size_t i = 0; i < size; ++i) {
        address_t current_address = start_addr + i;
        
        if (i % bytes_per_line == 0) {
            if (i > 0) {
                outfile << std::endl;
            }
            outfile << "0x" << std::hex << std::setw(8) << std::setfill('0') << current_address << ": ";
        }

        if (current_address < memory_.get_total_memory_size()) {
            outfile << std::hex << std::setw(2) << std::setfill('0') << (int)memory_.read_byte(current_address) << " ";
        } else {
            outfile << "?? ";
        }
    }
    outfile << std::endl;
    outfile.close();
}

void X86Simulator::dumpDataSegment(const std::string& filename) {
    const size_t active_memory_size = 256;
    dumpMemoryRange(filename, memory_.get_data_segment_start(), active_memory_size);
}

void X86Simulator::dumpBssSegment(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }
    outfile << "BSS segment dump not implemented." << std::endl;
    outfile.close();
}

void X86Simulator::dumpSymbolTable(const std::string& filename) {
      std::ofstream outputFile(filename);

    if (!outputFile) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    outputFile << "--- Symbol Table Dump ---\n";
    outputFile << std::left << std::setw(24) << "Symbol" << "Address\n";
    outputFile << "----------------------------------------\n";

    for (const auto& pair : symbolTable_) {
        std::string symbol = pair.first;
        outputFile << symbol;
        for (size_t i = symbol.length(); i < 24; ++i) {
            outputFile << ' ';
        }
        outputFile << "\t";

        outputFile << "0x" << std::hex << std::setw(8) << std::right << std::setfill('0') << pair.second << "\n";
    }
    outputFile.close();
}
