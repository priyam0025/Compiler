#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <optional>
#include <string>
#include <vector>
#include <filesystem>

#include "./generation.hpp"
#include "./parser.hpp"
#include "./tokenization.hpp"
#include "./arena.hpp"

int main(int argc, char* argv[]) {
    std::string filename;
    if (argc == 1) {
        filename = "test.hy";
    } else if (argc == 2) {
        filename = argv[1];
    } else {
        std::cerr << "Usage: hydro [input.hy]\n";
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::ifstream input(filename, std::ios::in);
        if (!input.is_open()) {
            std::cerr << "Failed to open file: " << filename << '\n';
            return EXIT_FAILURE;
        }
        std::ostringstream contents_stream;
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();
    if (tokens.empty()) {
        std::cerr << "No tokens produced; nothing to emit\n";
        return EXIT_FAILURE;
    }

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();
    if (!prog.has_value()) {
        std::cerr << "Invalid program\n";
        return EXIT_FAILURE;
    }

    Generator generator(prog.value());
    std::string asm_text = generator.gen_prog();

    namespace fs = std::filesystem;
    fs::path input_path = fs::absolute(filename);
    fs::path out_dir = input_path.parent_path();
    if (out_dir.empty()) out_dir = fs::current_path();

    fs::path out_asm_path = out_dir / "out.asm";
    fs::path out_o_path   = out_dir / "out.o";
    fs::path out_bin_path = out_dir / "out";

    // write asm (always truncate / overwrite)
    {
        std::ofstream file(out_asm_path, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            std::cerr << "Failed to open " << out_asm_path << " for writing\n";
            return EXIT_FAILURE;
        }
        file << asm_text;
        file.close();
    }

    // assemble + link (produce out.o and out), suppress command output
    {
        std::string nasm_cmd = "nasm -felf64 \"" + out_asm_path.string() + "\" -o \"" + out_o_path.string() + "\" > /dev/null 2>&1";
        int rc = std::system(nasm_cmd.c_str());
        if (rc != 0) {
            std::cerr << "nasm failed\n";
            return rc;
        }

        std::string ld_cmd = "ld -o \"" + out_bin_path.string() + "\" \"" + out_o_path.string() + "\" > /dev/null 2>&1";
        rc = std::system(ld_cmd.c_str());
        if (rc != 0) {
            std::cerr << "ld failed\n";
            return rc;
        }
    }

    return EXIT_SUCCESS;
}