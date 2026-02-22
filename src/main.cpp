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


// std::string tokens_to_asm(const std::vector<Token>& tokens) {
//     std::stringstream output;
//     output << "global _start\n_start:\n";
//     for (int i = 0; i < tokens.size(); i++) {
//         const Token& token = tokens.at(i);
//         if (token.type == TokenType::exit) {
//             if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit) {
//                 if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::semi) {
//                     output << "    mov rax, 60\n";
//                     output << "    mov rdi, " << tokens.at(i + 1).value.value() << '\n';
//                     output << "    syscall";
//                     i += 2;
//                 }
//             }
//         }
//     }
//     return output.str();
// }

// enum class TokenType {
//     exit,
//     int_lit,
//     semi
// };
// struct Token {
//     TokenType type;
//     std::optional<std::string> value;
// };

// std::vector<Token> tokenize(const std::string& str) {
//     const std::size_t n = str.size();
//     std::vector<Token> tokens;
//     std::string buf;
//     for (int i = 0; i < n; i++) 
//     {
//         // line comment start //
//         if (str[i] == '/' && i + 1 < n && str[i + 1] == '/') {
//             i += 2;
//             while (i < n && str[i] != '\n') ++i;
//             continue;
//         }
//         char c = str.at(i);
        
//         if (std::isalpha(c)) // identifier / keyword
//         {
//             buf.push_back(c);
//             i++;
//             while(std::isalpha(str.at(i)))
//             {
//                 buf.push_back(str.at(i));
//                 i++;
//             }
//             i--;
//             if (buf == "exit")
//             {
//                 tokens.push_back(Token{TokenType::exit, std::nullopt});
//                 buf.clear();
//                 continue;
//             }
//         }
       
//         else if (std::isdigit(c))  // integer literal
//         {
//             buf.push_back(c);
//             i++;
//             while(std::isdigit(str.at(i)))
//             {
//                 buf.push_back(str.at(i));
//                 i++;
//             }
//             i--;
//             tokens.push_back(Token{TokenType::int_lit, buf});
//             buf.clear();
//         }
//         else if (c == ';')  // semicolon
//         {
//             tokens.push_back(Token{TokenType::semi, std::nullopt});
//         }
//         else if (std::isspace(c))
//         {
//             continue;
//         }
//         else 
//         {
//             std::cerr << "You shit!" << std::endl;
//             exit(EXIT_FAILURE);
//         }
//     }
//     return tokens;
// }

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
    std::optional<node::NodeExit> tree = parser.parse();
    if (!tree.has_value()) {
        std::cerr << "No exit statement found" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.generate();
    }

    // const std::string asm_text = tokens_to_asm(tokens);
    // if (asm_text.empty()) {
    //     std::cerr << "Assembly output is empty; nothing to write\n";
    //     return EXIT_FAILURE;
    // }

    // {
    //     std::ofstream file("out.asm", std::ios::out | std::ios::trunc);
    //     if (!file.is_open()) {
    //         std::cerr << "Failed to open out.asm for writing\n";
    //         return EXIT_FAILURE;
    //     }
    //     file << asm_text;
    // }

    system("nasm -felf64 out.asm");
    system("ld -o out out.o");

    return EXIT_SUCCESS;
}