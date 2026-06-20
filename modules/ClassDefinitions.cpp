#ifndef CLASS_DEFINITIONS_CPP
#define CLASS_DEFINITIONS_CPP

#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <map>

class Interpreter {
    private:
        enum InstructionType {
            PRINT,
            NEW,
            SET,
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            INVALID
        };
        std::ifstream _sourceCodeFile;
        bool _errorFlag;
        void reportError(const std::string& message) { std::cerr << "Error: " << message << std::endl; }
        void reportWarning(const std::string& message) { std::cerr << "Warning: " << message << std::endl; }
        void reportInfo(const std::string& message) { std::cout << "Info: " << message << std::endl; }
        InstructionType getInstructionType(const std::string& instruction) {
            if (instruction.find("print") != std::string::npos) return InstructionType::PRINT;
            if (instruction.find("new") != std::string::npos) return InstructionType::NEW;
            if (instruction.find("set") != std::string::npos) return InstructionType::SET;
            if (instruction.find("add") != std::string::npos) return InstructionType::ADD;
            if (instruction.find("sub") != std::string::npos) return InstructionType::SUBTRACT;
            if (instruction.find("mul") != std::string::npos) return InstructionType::MULTIPLY;
            if (instruction.find("div") != std::string::npos) return InstructionType::DIVIDE;
            return InstructionType::INVALID;
        }
        public:
        Interpreter(const std::string& filename) : _sourceCodeFile(filename), _errorFlag(false) {
            if (!_sourceCodeFile.is_open()) {
                reportError("Failed to open source code file: " + filename);
                return;
            }
            reportInfo("Successfully opened source code file: " + filename);
        }
        ~Interpreter(void) { _sourceCodeFile.close(); }
        void interpret(void) {
            uint32_t lineNumber = 0;
            std::string line;
            std::map<std::string, uint32_t> variables;
            while (std::getline(_sourceCodeFile, line)) {
                lineNumber++;
                if (_errorFlag) return;
                if (line.empty()) continue;
                InstructionType instructionType = getInstructionType(line);
                std::vector<std::string> tokens;
                std::stringstream ss(line);
                std::string token;
                std::string variableName;
                while (ss >> token) { tokens.push_back(token); }
                try {
                    switch (instructionType) {
                        case InstructionType::PRINT:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            variableName = tokens[1];
                            std::cout << variables[variableName] << std::endl;
                            break;
                        case InstructionType::NEW:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            variables[tokens[1]] = 0;
                            break;
                        case InstructionType::SET:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            variables[tokens[1]] = std::stoi(tokens[2]);
                            break;
                        case InstructionType::ADD:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            variables[tokens[1]] += std::stoi(tokens[2]);
                            break;
                        case InstructionType::SUBTRACT:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            variables[tokens[1]] -= std::stoi(tokens[2]);
                            break;
                        case InstructionType::MULTIPLY:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            variables[tokens[1]] *= std::stoi(tokens[2]);
                            break;
                        case InstructionType::DIVIDE:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            variables[tokens[1]] /= std::stoi(tokens[2]);
                            break;
                        case InstructionType::INVALID:
                            reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                            _errorFlag = true;
                            break;
                    }
                } catch (...) {
                    reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                    _errorFlag = true;
                }
            }
        }
};

#endif
