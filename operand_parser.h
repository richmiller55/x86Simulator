#ifndef OPERAND_PARSER_H
#define OPERAND_PARSER_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

class OperandParser {
public:
    explicit OperandParser(const std::vector<std::string>& tokens) {
        if (tokens.size() > 1) {
            // Reconstruct the operand part of the line
            std::string args_str;
            for (size_t i = 1; i < tokens.size(); ++i) {
                args_str += tokens[i] + " ";
            }

            std::string current_operand;
            std::stringstream ss(args_str);

            // Split by comma
            while (std::getline(ss, current_operand, ',')) {
                // Trim whitespace
                current_operand.erase(0, current_operand.find_first_not_of(" \t\n\r"));
                current_operand.erase(current_operand.find_last_not_of(" \t\n\r") + 1);
                std::transform(current_operand.begin(), current_operand.end(), current_operand.begin(), ::tolower);
                if (!current_operand.empty()) {
                    operands_.push_back(current_operand);
                }
            }
        }
    }

    size_t operand_count() const { return operands_.size(); }

    std::string get_operand(size_t index) const {
        return (index < operands_.size()) ? operands_[index] : "";
    }

private:
    std::vector<std::string> operands_;
};

#endif // OPERAND_PARSER_H