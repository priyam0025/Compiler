#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <cstdlib>

enum class TokenType { exit, int_lit, semi, open_paren, close_paren, ident, let, eq};
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

            while (peek().has_value()) 
            {
                char c = peek().value();
                if (std::isalpha(static_cast<unsigned char>(c))) {
                    buf.clear();
                    buf.push_back(consume());
                    while (peek().has_value() && std::isalpha(static_cast<unsigned char>(peek().value()))) { 
                        buf.push_back(consume());
                    }
                    if (buf == "exit") {
                        tokens.push_back(Token{TokenType::exit, std::nullopt});
                        continue;
                    }
                    else if (buf == "let") {
                        tokens.push_back(Token{TokenType::let, std::nullopt});
                        continue;
                    } 
                    else {
                        tokens.push_back(Token{TokenType::ident, buf});
                        continue;
                    }
                }
                else if (std::isdigit(static_cast<unsigned char>(c))) {
                    buf.clear();
                    buf.push_back(consume());
                    while (peek().has_value() && std::isdigit(static_cast<unsigned char>(peek().value()))) {
                        buf.push_back(consume());
                    }
                    tokens.push_back(Token{TokenType::int_lit, buf});
                    continue;
                }
                else if (c == ')') {
                    consume();
                    tokens.push_back(Token{TokenType::close_paren, std::nullopt});
                    continue;
                }
                else if (c == '(') {
                    consume();
                    tokens.push_back(Token{TokenType::open_paren, std::nullopt});
                    continue;
                }
                else if (c == ';') {
                    consume();
                    tokens.push_back(Token{TokenType::semi, std::nullopt});
                    continue;
                }
                else if (c == '=') {
                    consume();
                    tokens.push_back(Token{TokenType::eq, std::nullopt});
                    continue;
                }
                else if (std::isspace(static_cast<unsigned char>(c))) {
                    consume();
                    continue;
                }
                else {
                    std::cerr << "Unexpected character in input: '" << c << "'" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
            }

            return tokens;
        }
    private:
        inline std::optional<char> peek(std::size_t offset = 0) const 
        {
            if (m_index + offset >= m_src.length()) {
                return {};
            } else {
                return m_src.at(m_index + offset);
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