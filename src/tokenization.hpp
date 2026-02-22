#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <cstdlib>

enum class TokenType {
    exit,
    int_lit,
    semi
};
struct Token {
    TokenType type;
    std::optional<std::string> value;
};
class Tokenizer {
    public:
        inline explicit Tokenizer(std::string src) 
            : m_src(std::move(src))
        {
        }
        inline std::vector<Token> tokenize() {
            
            std::vector<Token> tokens;
            std::string buf;

            while (peak().has_value()) 
            {
                if (std::isalpha(peak().value())) {
                    buf.push_back(consume());
                    while (peak().has_value() && std::isalpha(peak().value())) { 
                        buf.push_back(consume());
                    }
                    if (buf =="exit") {
                        tokens.push_back(Token{TokenType::exit, std::nullopt});
                        buf.clear();
                        continue;
                    } else {
                        std::cerr << "You shit1!" << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
                else if (std::isdigit(peak().value())) {
                    buf.push_back(consume());
                    while (peak().has_value() && std::isdigit(peak().value())) {
                        buf.push_back(consume());
                    }
                    tokens.push_back(Token{TokenType::int_lit, buf});
                    buf.clear();
                    continue;
                }
                else if (peak().value() == ';') {
                    consume();
                    tokens.push_back(Token{TokenType::semi, std::nullopt});
                    continue;
                }
                else if (std::isspace(peak().value())) {
                    consume();
                    continue;
                }
                else {
                    std::cerr << "You shit2!" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            return tokens;
        }
    private:
        inline std::optional<char> peak(std::size_t ahead = 0) const 
        {
            if (m_index + ahead >= m_src.length()) {
                return {};
            } else {
                return m_src.at(m_index + ahead);
            }
        }

        inline char consume() 
        {
            if (m_index >= m_src.size()) return '\0';
            return m_src.at(m_index++);
        }

        const std::string m_src;
        std::size_t m_index = 0;
};