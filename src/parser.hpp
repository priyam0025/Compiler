#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <cctype>
#include <cstdlib>
#include <variant>
#include "./tokenization.hpp"
#include "./arena.hpp"

// forward-declare NodeExpr so bin-expr structs can refer to it
struct NodeExpr;

struct NodeTermIntLit
{
    Token int_lit;
};

struct NodeTermIdent
{
    Token ident;
};

struct NodeBinExprMulti
{
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprAdd
{
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr
{
    std::variant<NodeBinExprMulti *, NodeBinExprAdd *> var;
};

struct NodeTerm
{
    std::variant<NodeTermIntLit *, NodeTermIdent *> var;
};

struct NodeExpr
{
    std::variant<NodeTermIntLit *, NodeTermIdent *, NodeBinExpr *> var;
};

struct NodeStmtExit
{
    NodeExpr *expr;
};

struct NodeStmtLet
{
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt
{
    std::variant<NodeStmtExit *, NodeStmtLet *> var;
};

struct NodeProg
{
    std::vector<NodeStmt *> stmts;
};

class Parser
{
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens)), m_allocator(1024 * 1024 * 4) // 4 MB
    {
    }

    // placeholder for future binary-expression parsing
    // std::optional<NodeBinExpr*> parse_bin_expr() { ... }

    std::optional<NodeTerm *> parse_term()
    {
        auto p = peek();
        if (p.has_value() && p->type == TokenType::int_lit)
        {
            Token t = consume();
            auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
            term_int_lit->int_lit = t;
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_int_lit;
            return term;
        }
        else if (p.has_value() && p->type == TokenType::ident)
        {
            Token t = consume();
            auto term_ident = m_allocator.alloc<NodeTermIdent>();
            term_ident->ident = t;
            auto term = m_allocator.alloc<NodeTerm>();
            term->var = term_ident;
            return term;
        }
        else
        {
            return {};
        }
    }

    std::optional<NodeExpr *> parse_expr()
    {
        // Try to parse a term first (either an integer literal or an identifier)
        if (auto term = parse_term())
        {

            // parse_term() returns NodeTerm*, so store it for easier access
            NodeTerm *term_ptr = term.value();

            // Check if the next token is a '+' operator
            // If yes, we need to build a binary addition expression
            if (peek().has_value() && peek().value().type == TokenType::plus)
            {

                // Allocate memory for a binary expression node
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();

                // Allocate memory specifically for an addition expression
                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();

                // Create a NodeExpr that will become the left-hand side (lhs)
                auto lhs_expr = m_allocator.alloc<NodeExpr>();

                // NodeTerm contains a variant (either int literal or identifier)
                // We must extract the correct type and assign it to NodeExpr
                if (auto int_lit = std::get_if<NodeTermIntLit *>(&term_ptr->var))
                {
                    lhs_expr->var = *int_lit;
                }
                else if (auto ident = std::get_if<NodeTermIdent *>(&term_ptr->var))
                {
                    lhs_expr->var = *ident;
                }

                // Set the left-hand side of the addition expression
                bin_expr_add->lhs = lhs_expr;

                // Consume the '+' token
                consume();

                // Recursively parse the right-hand side expression
                if (auto rhs = parse_expr())
                {

                    // Set the right-hand side of the addition
                    bin_expr_add->rhs = rhs.value();

                    // Store the addition node inside the binary expression variant
                    bin_expr->var = bin_expr_add;

                    // Wrap the binary expression inside a NodeExpr
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = bin_expr;

                    // Return the final expression node
                    return expr;
                }
                else
                {
                    // Error if the RHS expression is missing
                    std::cerr << "Expected Expression" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
            }
            else
            {
                // If there is no '+' operator,
                // this expression is simply a single term

                auto expr = m_allocator.alloc<NodeExpr>();

                // Extract the term type and assign it to NodeExpr
                if (auto int_lit = std::get_if<NodeTermIntLit *>(&term_ptr->var))
                {
                    expr->var = *int_lit;
                }
                else if (auto ident = std::get_if<NodeTermIdent *>(&term_ptr->var))
                {
                    expr->var = *ident;
                }

                // Return the single-term expression
                return expr;
            }
        }
        else
        {
            // If no term could be parsed, return empty optional
            return {};
        }
    }

    std::optional<NodeStmt *> parse_stat()
    {
        while (peek().has_value())
        {
            // exit(expr);
            if (peek()->type == TokenType::exit && peek(1).has_value() && peek(1)->type == TokenType::open_paren)
            {
                consume(); // consume 'exit'
                consume(); // consume '('
                auto node_expr = parse_expr();
                if (!node_expr)
                {
                    std::cerr << "Invalid expression after exit(" << std::endl;
                    std::exit(EXIT_FAILURE);
                }
                if (!peek().has_value() || peek()->type != TokenType::close_paren)
                {
                    std::cerr << "Expected ')' after expression\n";
                    std::exit(EXIT_FAILURE);
                }
                consume(); // consume ')'
                if (!peek().has_value() || peek()->type != TokenType::semi)
                {
                    std::cerr << "Expected ';' after exit(...) statement\n";
                    std::exit(EXIT_FAILURE);
                }
                consume(); // consume ';'

                auto node_stmt_exit = m_allocator.alloc<NodeStmtExit>();
                if (!node_stmt_exit)
                    std::exit(EXIT_FAILURE);
                node_stmt_exit->expr = node_expr.value();

                auto node_stmt = m_allocator.alloc<NodeStmt>();
                if (!node_stmt)
                    std::exit(EXIT_FAILURE);
                node_stmt->var = node_stmt_exit;
                return node_stmt;
            }
            // let <ident> = expr;
            else if (peek()->type == TokenType::let && peek(1).has_value() &&
                     peek(1)->type == TokenType::ident && peek(2).has_value() && peek(2)->type == TokenType::eq)
            {
                consume();                     // consume 'let'
                Token ident_token = consume(); // consume identifier
                consume();                     // consume '='
                auto expr = parse_expr();
                if (!expr)
                {
                    std::cerr << "Invalid expression in let statement\n";
                    std::exit(EXIT_FAILURE);
                }
                if (!peek().has_value() || peek()->type != TokenType::semi)
                {
                    std::cerr << "Expected ';' after let statement\n";
                    std::exit(EXIT_FAILURE);
                }
                consume(); // consume ';'

                auto node_stmt_let = m_allocator.alloc<NodeStmtLet>();
                if (!node_stmt_let)
                    std::exit(EXIT_FAILURE);
                node_stmt_let->ident = ident_token;
                node_stmt_let->expr = expr.value();

                auto node_stmt = m_allocator.alloc<NodeStmt>();
                if (!node_stmt)
                    std::exit(EXIT_FAILURE);
                node_stmt->var = node_stmt_let;
                return node_stmt;
            }
            else
            {
                return {};
            }
        }
        return {};
    }

    std::optional<NodeProg> parse_prog()
    {
        NodeProg prog;
        while (peek().has_value())
        {
            auto stmt = parse_stat();
            if (stmt)
            {
                prog.stmts.push_back(stmt.value());
            }
            else
            {
                std::cerr << "Invalid statement in program\n";
                std::exit(EXIT_FAILURE);
            }
        }
        return prog;
    }

private:
    inline std::optional<Token> peek(std::size_t offset = 0) const
    {
        if (m_index + offset >= m_tokens.size())
        {
            return {};
        }
        else
        {
            return m_tokens.at(m_index + offset);
        }
    }

    inline Token consume()
    {
        if (m_index >= m_tokens.size())
        {
            std::cerr << "Consume called past end of token stream\n";
            std::exit(EXIT_FAILURE);
        }
        return m_tokens.at(m_index++);
    }

    std::size_t m_index = 0;
    std::vector<Token> m_tokens;
    ArenaAllocator m_allocator;
};