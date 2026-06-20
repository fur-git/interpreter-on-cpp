#include "modules/ClassDefinitions.cpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source-file>\n";
        return 1;
    }
    Interpreter interpreter(argv[1]);
    interpreter.interpret();
    return 0;
}
