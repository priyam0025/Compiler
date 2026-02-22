#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    _return,
    int_lit,
    semi
};
struct Token {
    TokenType type;
    std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string& str) {
    const std::size_t n = str.size();
    std::vector<Token> tokens;
    std::string buf;
    for (int i = 0; i < n; i++) 
    {
        // line comment start //
        if (str[i] == '/' && i + 1 < n && str[i + 1] == '/') {
            i += 2;
            while (i < n && str[i] != '\n') ++i;
            continue;
        }
        char c = str.at(i);
        
        if (std::isalpha(c)) // identifier / keyword
        {
            buf.push_back(c);
            i++;
            while(std::isalpha(str.at(i)))
            {
                buf.push_back(str.at(i));
                i++;
            }
            i--;
            if (buf == "return")
            {
                tokens.push_back(Token{TokenType::_return, std::nullopt});
                buf.clear();
                continue;
            }
        }
       
        else if (std::isdigit(c))  // integer literal
        {
            buf.push_back(c);
            i++;
            while(std::isdigit(str.at(i)))
            {
                buf.push_back(str.at(i));
                i++;
            }
            i--;
            tokens.push_back(Token{TokenType::int_lit, buf});
            buf.clear();
        }
        else if (c == ';')  // semicolon
        {
            tokens.push_back(Token{TokenType::semi, std::nullopt});
        }
        else if (std::isspace(c))
        {
            continue;
        }
        else 
        {
            std::cerr << "You shit!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    return tokens;
}

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
    std::vector<Token> thing = tokenize(contents);
    

    return EXIT_SUCCESS;
}