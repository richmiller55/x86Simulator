#ifndef OPERAND_PARSER_H
#define OPERAND_PARSER_H

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

class OperandParser {
public:
    explicit OperandParser(const std::vector<std::string>& tokens) {
        for (size_t i = 1; i < tokens.size(); ++i) {
            std::string token = tokens[i];
            if (token != ",") {
                std::transform(token.begin(), token.end(), token.begin(), ::tolower);
                operands_.push_back(token);
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