#ifndef CLASS_DEFINITIONS_CPP
#define CLASS_DEFINITIONS_CPP

#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <sstream>
#include <format>
#include <cstdlib>
#include <map>

class AssemblyCode {
    private:
        std::ofstream _assemblyCodeFile;
        struct AssemblyTextSection {
                std::string _assemblyTextSection;
                AssemblyTextSection(const std::string& beginningOfSection) { _assemblyTextSection = beginningOfSection; }
                void addInstruction(const std::string& instruction) { _assemblyTextSection += instruction; }
                std::string getAssemblyTextSection(void) { return _assemblyTextSection; }
        };
        struct AssemblyDataSection {
            std::string _assemblyDataSection;
            AssemblyDataSection(const std::string& beginningOfSection) { _assemblyDataSection = beginningOfSection; }
            void addInstruction(const std::string& instruction) { _assemblyDataSection += instruction; }
            std::string getAssemblyDataSection(void) { return _assemblyDataSection; }
        };
        struct AssemblyBssSection {
            std::string _assemblyBssSection;
            AssemblyBssSection(const std::string& beginningOfSection) { _assemblyBssSection = beginningOfSection; }
            void addInstruction(const std::string& instruction) { _assemblyBssSection += instruction; }
            std::string getAssemblyBssSection(void) { return _assemblyBssSection; }
        };
        struct AssemblyFunctions {
            std::string _assemblyFunctions;
            AssemblyFunctions(const std::string& beginningOfSection) { _assemblyFunctions = beginningOfSection; }
            void addInstruction(const std::string& instruction) { _assemblyFunctions += instruction; }
            std::string getAssemblyFunctions(void) { return _assemblyFunctions; }
        };
        AssemblyTextSection _assemblyTextSection{".section .text\n.global _start\n\n_start:"};
        AssemblyDataSection _assemblyDataSection{".section .data\n"};
        AssemblyBssSection _assemblyBssSection{"\n.section .bss\n\nbuffer: .zero 33\n"};
        AssemblyFunctions _assemblyFunctions{"\n.section .text\n"};
    public:
        AssemblyCode(const std::string& filename) {
            _assemblyCodeFile.open(filename.substr(0, filename.find_last_of('.')) + ".S");
        }
        ~AssemblyCode(void) {
            _assemblyCodeFile.close();
        }
        void addInstructionToText(const std::string& instruction) { _assemblyTextSection.addInstruction(instruction); }
        void addInstructionToData(const std::string& instruction) { _assemblyDataSection.addInstruction(instruction); }
        void addInstructionToBss(const std::string& instruction) { _assemblyBssSection.addInstruction(instruction); }
        void addInstructionToFunctions(const std::string& instruction) { _assemblyFunctions.addInstruction(instruction); }
        std::string getAssemblyTextSection(void) { return _assemblyTextSection.getAssemblyTextSection(); }
        std::string getAssemblyDataSection(void) { return _assemblyDataSection.getAssemblyDataSection(); }
        std::string getAssemblyBssSection(void) { return _assemblyBssSection.getAssemblyBssSection(); }
        std::string getAssemblyFunctions(void) { return _assemblyFunctions.getAssemblyFunctions(); }
        void writeToFile(void) {
            _assemblyCodeFile
            << _assemblyDataSection.getAssemblyDataSection()
            << _assemblyBssSection.getAssemblyBssSection()
            << _assemblyFunctions.getAssemblyFunctions()
            << _assemblyTextSection.getAssemblyTextSection()
            << std::endl; 
        }
};

class Compiler {
    private:
        enum InstructionType {
            PRINT,
            NEW,
            SET,
            ADD,
            SUBTRACT,
            MULTIPLY,
            DIVIDE,
            NEWLINE,
            EXIT,
            INVALID
        };
        std::ifstream _sourceCodeFile;
        AssemblyCode _assemblyCode;
        std::string _filename;
        bool _exitInstructionFlag;
        bool _is64Bits;
        InstructionType getInstructionType(const std::string& instruction) {
            if (instruction.find("print") != std::string::npos) return InstructionType::PRINT;
            if (instruction.find("newline") != std::string::npos) return InstructionType::NEWLINE;
            if (instruction.find("new") != std::string::npos) return InstructionType::NEW;
            if (instruction.find("set") != std::string::npos) return InstructionType::SET;
            if (instruction.find("add") != std::string::npos) return InstructionType::ADD;
            if (instruction.find("subtract") != std::string::npos) return InstructionType::SUBTRACT;
            if (instruction.find("multiply") != std::string::npos) return InstructionType::MULTIPLY;
            if (instruction.find("divide") != std::string::npos) return InstructionType::DIVIDE;
            if (instruction.find("exit") != std::string::npos) return InstructionType::EXIT;
            return InstructionType::INVALID;
        }
        bool doesTheVariableExist(const std::string& variableName) {
            return _assemblyCode.getAssemblyDataSection().find(variableName) != std::string::npos;
        }
    public:
        bool _errorFlag;
        static void reportError(const std::string& message) { std::cerr << "Error: " << message << std::endl; }
        static void reportWarning(const std::string& message) { std::cerr << "Warning: " << message << std::endl; }
        static void reportInfo(const std::string& message) { std::cout << "Info: " << message << std::endl; }
        Compiler(const std::string& filename, bool is64Bits) : _assemblyCode(filename) {
            _filename = filename;
            _sourceCodeFile.open(filename);
            _errorFlag = false;
            _exitInstructionFlag = false;
            _is64Bits = is64Bits;
            if (!_sourceCodeFile.is_open()) {
                reportError("Failed to open source code file: " + filename);
                return;
            }
            reportInfo("Successfully opened source code file: " + filename);
        }
        ~Compiler(void) {
            _sourceCodeFile.close();
        }
        void compile(void) {
            std::ostringstream sourceBuffer;
            sourceBuffer << _sourceCodeFile.rdbuf();
            std::string source = sourceBuffer.str();
            _sourceCodeFile.clear();
            _sourceCodeFile.seekg(0, std::ios::beg);
            uint32_t lineNumber = 0;
            std::string line;
            if (source.find("print") != std::string::npos) {
                const std::string itoaAssemblyCode = R"(
itoa:
    testq %rax, %rax
    jne itoa_not_zero
    movb $'0', buffer(%rip)
    movb $0, buffer+1(%rip)
    ret
itoa_not_zero:
    movq %rax, %r8
    xorq %r9, %r9
    xorq %r10, %r10
    jns itoa_positive
    negq %r8
    movq $1, %r10
itoa_positive:
    leaq buffer+31(%rip), %rcx
itoa_loop:
    xorq %rdx, %rdx
    movq %r8, %rax
    movq $10, %rdi
    divq %rdi
    movq %rax, %r8
    addb $'0', %dl
    decq %rcx
    movb %dl, (%rcx)
    incq %r9
    testq %r8, %r8
    jnz itoa_loop
    cmpq $0, %r10
    je itoa_done
    decq %rcx
    movb $'-', (%rcx)
    incq %r9
itoa_done:
    movb $0, (%rcx, %r9, 1)
    movq %r9, %r11
    movq %rcx, %rsi
    leaq buffer(%rip), %rdi
    movq %r9, %rcx
    rep movsb
    leaq buffer(%rip), %rdi
    addq %r11, %rdi
    movb $0, (%rdi)
    ret

get_string_length:
    movq $0, %rcx
get_string_length_loop:
    cmpb $0, (%rax)
    je get_string_length_done
    incq %rcx
    incq %rax
    jmp get_string_length_loop
get_string_length_done:
    ret

)";
                _assemblyCode.addInstructionToFunctions(itoaAssemblyCode);
            }
            if (source.find("newline") != std::string::npos) {
                const std::string newlineAssemblyCode = R"(
    newline: .ascii "\n"
)";
                _assemblyCode.addInstructionToData(newlineAssemblyCode);
            }
            while (std::getline(_sourceCodeFile, line)) {
                lineNumber++;
                if (_errorFlag) { return; }
                if (line.empty()) { continue; }
                InstructionType instructionType = getInstructionType(line);
                std::vector<std::string> tokens;
                std::stringstream ss(line);
                std::string token;
                std::string variableName;
                while (ss >> token) { tokens.push_back(token); }
                try {
                    std::string assemblyInstruction;
                    switch (instructionType) {
                        case InstructionType::PRINT:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    call itoa
    leaq buffer(%rip), %rax
    call get_string_length
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    syscall
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movslq {}(%rip), %rax
    call itoa
    leaq buffer(%rip), %rax
    call get_string_length
    movq %rcx, %rdx
    movq $1, %rax
    movq $1, %rdi
    leaq buffer(%rip), %rsi
    syscall
)", tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::NEW:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " already exists");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    {}: .quad 0
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    {}: .long 0
)", tokens[1]);
                            }
                            _assemblyCode.addInstructionToData(assemblyInstruction);
                            break;
                        case InstructionType::SET:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq ${}, {}(%rip)
)", tokens[2], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl ${}, {}(%rip)
)", tokens[2], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::ADD:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    addq {}(%rip), {}(%rip)
)", tokens[2], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    addl {}(%rip), {}(%rip)
)", tokens[2], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::SUBTRACT:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    subq {}(%rip), {}(%rip)
)", tokens[2], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    subl {}(%rip), {}(%rip)
)", tokens[2], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::MULTIPLY:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    movq {}(%rip), %rbx
    imulq %rax, %rbx
    movq %rbx, {}(%rip)
)", tokens[2], tokens[1], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    movl {}(%rip), %ebx
    imull %eax, %ebx
    movl %ebx, {}(%rip)
)", tokens[2], tokens[1], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::DIVIDE:
                            if (tokens.size() != 3) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq {}(%rip), %rax
    cqto
    movq {}(%rip), %rbx
    idivq %rbx
    movq %rax, {}(%rip)
)", tokens[2], tokens[1], tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl {}(%rip), %eax
    cltd
    movl {}(%rip), %ebx
    idivl %ebx
    movl %eax, {}(%rip)
)", tokens[2], tokens[1], tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::NEWLINE:
                            if (tokens.size() != 1) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            assemblyInstruction = R"(
    movq $1, %rax
    movq $1, %rdi
    leaq newline(%rip), %rsi
    movq $1, %rdx
    syscall
)";
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            break;
                        case InstructionType::EXIT:
                            if (tokens.size() != 2) {
                                reportError("Invalid instruction: " + line + " at line " + std::to_string(lineNumber));
                                _errorFlag = true;
                                continue;
                            }
                            if (!doesTheVariableExist(tokens[1])) {
                                reportError("Variable " + tokens[1] + " does not exist");
                                _errorFlag = true;
                                continue;
                            }
                            if (_is64Bits) {
                                assemblyInstruction = std::format(R"(
    movq $60, %rax
    movslq {}(%rip), %rdi
    syscall
)", tokens[1]);
                            } else {
                                assemblyInstruction = std::format(R"(
    movl $60, %eax
    movslq {}(%rip), %edi
    syscall
)", tokens[1]);
                            }
                            _assemblyCode.addInstructionToText(assemblyInstruction);
                            _exitInstructionFlag = true;
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
            if (!_exitInstructionFlag) {
                std::string finalInstruction = R"(
    movq $60, %rax
    xorq %rdi, %rdi
    syscall
)";
                _assemblyCode.addInstructionToText(finalInstruction);
            }
            _assemblyCode.writeToFile();
            if (_errorFlag) { return; }
            std::string filenameWithoutExtension = _filename.substr(0, _filename.find_last_of('.'));
            if (system(std::format("as {}.S -o {}.o", filenameWithoutExtension, filenameWithoutExtension).c_str()) != 0) {
                reportError("Assembler failed");
                _errorFlag = true;
                return;
            }
            if (system(std::format("ld {}.o -o {}", filenameWithoutExtension, filenameWithoutExtension).c_str()) != 0) {
                reportError("Linker failed");
                _errorFlag = true;
            }
        }
};

#endif
