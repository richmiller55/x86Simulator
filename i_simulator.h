#ifndef I_SIMULATOR_H
#define I_SIMULATOR_H

class IRInstruction; // Forward declaration

#include <string>

// Forward declarations for components
class RegisterMap;
class Memory;
class IDatabaseManager;
class Architecture;
class IRVisitor;

/**
 * @class ISimulator
 * @brief An interface for a generic instruction set simulator.
 *
 * This abstract base class defines the common contract that all concrete
 * simulator implementations (e.g., X86Simulator, ArmSimulator) must follow.
 * This allows the SystemBus to manage and run different types of simulators
 * without knowing their specific ISA.
 */
class ISimulator {
public:
    // Virtual destructor is required for a polymorphic base class.
    virtual ~ISimulator() = default;

    /**
     * @brief Accepts a visitor to perform an operation on the simulator.
     * @param visitor The visitor to accept.
     */
    virtual void accept(IRVisitor& visitor, const IRInstruction& instr) = 0;

    /**
     * @brief Starts the main execution loop of the simulator.
     */
    virtual void runProgram() = 0;

    /**
     * @brief Loads a program from the specified path into the simulator's memory.
     * @param program_path The file path to the program to be loaded.
     * @return True if loading was successful, false otherwise.
     */
    virtual bool loadProgram(const std::string& program_path) = 0;

    // --- Generic Component Getters ---
    virtual RegisterMap& getRegisterMap() = 0;
    virtual const RegisterMap& getRegisterMap() const = 0;
    virtual Memory& getMemory() = 0;
    virtual const Memory& getMemory() const = 0;
    virtual IDatabaseManager& getDatabaseManager() = 0;
    virtual const Architecture& get_architecture() const = 0;
    virtual int get_session_id() const = 0;

    // --- Generic Flag Setters/Getters ---
    // These abstract away the details of specific flag registers like RFLAGS or CPSR.
    virtual void set_ZF(bool value) = 0;
    virtual void set_SF(bool value) = 0;
    virtual void set_CF(bool value) = 0;
    virtual void set_OF(bool value) = 0;
    virtual void set_PF(bool value) = 0;

    virtual bool get_ZF() const = 0;
    virtual bool get_SF() const = 0;
    virtual bool get_CF() const = 0;
    virtual bool get_OF() const = 0;
    virtual bool get_PF() const = 0;
};

#endif // I_SIMULATOR_H
