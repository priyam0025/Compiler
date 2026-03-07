#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <optional>
#include <string>
#include <vector>

#include "./generation.hpp"
#include "./parser.hpp"
#include "./tokenization.hpp"
#include "./arena.hpp"

int main(int argc, char* argv[]) {
    std::string filename;
    if (argc == 1) {
        filename = "test.hy";            // default file
    } else if (argc == 2) {
        filename = argv[1];
    } else {
        std::cerr << "Incorrect usage. Correct usage is:" << std::endl;
        std::cerr << "hydro [input.hy]" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::ifstream input(filename, std::ios::in);
        if (!input.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return EXIT_FAILURE;
        }

        std::ostringstream contents_stream;
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }
    
    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();
    if (tokens.empty()) {
        std::cerr << "No tokens produced; nothing to emit to out.asm\n";
        return EXIT_FAILURE;
    }

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();
    if (!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(prog.value());
    {
        std::string asm_text = generator.gen_prog();
        std::ofstream file("out.asm", std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Failed to open out.asm for writing\n";
            return EXIT_FAILURE;
        }
        file << asm_text;
        file.close();

        if (asm_text.empty()) {
            std::cerr << "Generated assembly is empty\n";
            return EXIT_FAILURE;
        }
    }

    int rc = std::system("nasm -felf64 out.asm");
    if (rc != 0) {
        std::cerr << "nasm failed with code: " << rc << std::endl;
        return rc;
    }
    rc = std::system("ld -o out out.o");
    if (rc != 0) {
        std::cerr << "ld failed with code: " << rc << std::endl;
        return rc;
    }

    return EXIT_SUCCESS;
}