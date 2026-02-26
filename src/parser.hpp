#pragma once 

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <cstdlib>
#include <variant>
#include "./tokenization.hpp"

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr {
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtLet> var;
};

struct NodeProg {
    std::vector<NodeStmt> stmts;
};

class Parser {
    public:
        inline explicit Parser (std::vector<Token> tokens)
            : m_tokens(std::move(tokens))
        {
        }

        std::optional<NodeExpr> parse_expr() {
            auto p = peek();
            if (p.has_value() && p->type == TokenType::int_lit) {
                Token t = consume();
                return NodeExpr {.var = NodeExprIntLit{.int_lit = t}};
            }
            else if (p.has_value() && p->type == TokenType::ident) {
                Token t = consume();
                return NodeExpr {.var = NodeExprIdent{.ident = t}};
            } 
            else {
                return {};
            }
        }

        std::optional<NodeStmt> parse_stat() {
            while(peek().has_value()) 
            {
                // exit(expr);
                if (peek()->type == TokenType::exit && peek(1).has_value() 
                && peek(1)->type == TokenType::open_paren) 
                {
                    consume(); // consume 'exit'
                    consume(); // consume '('
                    auto node_expr = parse_expr();
                    if (!node_expr) {
                        std::cerr << "Invalid expression after exit(" << std::endl;
                        std::exit(EXIT_FAILURE);
                    }
                    if (!peek().has_value() || peek()->type != TokenType::close_paren) {
                        std::cerr << "Expected ')' after expression\n";
                        std::exit(EXIT_FAILURE);
                    }
                    consume(); // consume ')'
                    if (!peek().has_value() || peek()->type != TokenType::semi) {
                        std::cerr << "Expected ';' after exit(...) statement\n";
                        std::exit(EXIT_FAILURE);
                    }
                    consume(); // consume ';'
                    NodeStmtExit st{.expr = node_expr.value()};
                    return NodeStmt{.var = st};
                }
                // let <ident> = expr;
                else if (peek()->type == TokenType::let && peek(1).has_value() &&
                peek(1)->type == TokenType::ident && peek(2).has_value() && peek(2)->type == TokenType::eq) {
                    consume(); // consume 'let'
                    Token ident_token = consume(); // consume identifier
                    consume(); // consume '='
                    auto expr = parse_expr();
                    if (!expr) {
                        std::cerr << "Invalid expression in let statement\n";
                        std::exit(EXIT_FAILURE);
                    }
                    if (!peek().has_value() || peek()->type != TokenType::semi) {
                        std::cerr << "Expected ';' after let statement\n";
                        std::exit(EXIT_FAILURE);
                    }
                    consume(); // consume ';'
                    NodeStmtLet st{ .ident = ident_token, .expr = expr.value() };
                    return NodeStmt{.var = st};
                } else {
                    return {};
                }
            }
            return {};
        }

        std::optional<NodeProg> parse_prog() {
            NodeProg prog;
            while (peek().has_value()) {
                auto stmt = parse_stat();
                if (stmt) {
                    prog.stmts.push_back(stmt.value());
                } 
                else {
                    std::cerr << "Invalid statement in program\n";
                    std::exit(EXIT_FAILURE);
                }
            }
            return prog;
        }

    private:
        inline std::optional<Token> peek(std::size_t offset = 0) const 
        {
            if (m_index + offset >= m_tokens.size()) {
                return {};
            } else {
                return m_tokens.at(m_index + offset);
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