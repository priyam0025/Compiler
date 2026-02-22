#pragma once 

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <cstdlib>
#include "./tokenization.hpp"

namespace node {
    struct NodeExpr {
        Token int_lit;
    };

    struct NodeExit {
        NodeExpr expr;
    };
}

class Parser {
    public:
        inline explicit Parser (std::vector<Token> tokens)
            : m_tokens(std::move(tokens))
        {
        }

        std::optional<node::NodeExpr> parse_expr() {
            auto p = peak();
            if (p.has_value() && p->type == TokenType::int_lit) {
                Token t = consume();
                return node::NodeExpr{t};
            } else {
                return {};
            }
        }

        std::optional<node::NodeExit> parse() 
        {
            std::optional<node::NodeExit> exit_node;
            while(peak().has_value()) {
                if (peak().value().type == TokenType::exit) {
                    consume();
                    if (auto node_expr = parse_expr()) {
                        exit_node = node::NodeExit{.expr = node_expr.value()};
                    } else {
                        std::cerr << "Invalid Expression" << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    auto semi = peak();
                    if (!semi.has_value() || semi->type != TokenType::semi) {
                        std::cerr << "Expected ';' after expression\n";
                        std::exit(EXIT_FAILURE);
                    }
                    consume(); // consume ';'

                    continue;
                }

                std::cerr << "Unexpected token at top-level\n";
                std::exit(EXIT_FAILURE);
            }

            m_index = 0;
            return exit_node;
        }

    private:
        inline std::optional<Token> peak(std::size_t ahead = 0) const 
        {
            if (m_index + ahead >= m_tokens.size()) {
                return {};
            } else {
                return m_tokens.at(m_index + ahead);
            }
        }

        inline Token consume() 
        {
            if (m_index >= m_tokens.size()) {
                std::cerr << "Consume called past end of token stream\n";
                std::exit(EXIT_FAILURE);
            }
            return m_tokens.at(m_index++);
        }

        std::size_t m_index = 0;
        std::vector<Token> m_tokens;
};