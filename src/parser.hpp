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

struct NodeBinExprSub
{
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExprDiv
{
    NodeExpr *lhs;
    NodeExpr *rhs;
};

struct NodeBinExpr
{
    std::variant<NodeBinExprMulti *, NodeBinExprAdd *, NodeBinExprSub *, NodeBinExprDiv *> var;
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

struct NodeStmtAssign
{
    Token ident;
    NodeExpr *expr;
};

struct NodeStmt;

struct NodeStmtBlock
{
    std::vector<NodeStmt *> stmts;
};

struct NodeStmtIf
{
    NodeExpr *cond;
    NodeStmt *then_stmt;
    std::optional<NodeStmt *> else_stmt;
};

struct NodeStmt
{
    std::variant<NodeStmtExit *, NodeStmtLet *, NodeStmtAssign *, NodeStmtBlock *, NodeStmtIf *> var;
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

    std::optional<NodeExpr *> parse_primary_expr()
    {
        auto p = peek();
        if (!p.has_value()) return {};

        if (p->type == TokenType::int_lit) {
            Token t = consume();
            auto lit = m_allocator.alloc<NodeTermIntLit>();
            lit->int_lit = t;
            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = lit;
            return expr;
        }
        else if (p->type == TokenType::ident) {
            Token t = consume();
            auto id = m_allocator.alloc<NodeTermIdent>();
            id->ident = t;
            auto expr = m_allocator.alloc<NodeExpr>();
            expr->var = id;
            return expr;
        }
        else if (p->type == TokenType::open_paren) {
            consume(); // '('
            auto expr = parse_expr();
            if (!expr) {
                std::cerr << "Expected expression after '('\n";
                std::exit(EXIT_FAILURE);
            }
            if (!peek().has_value() || peek()->type != TokenType::close_paren) {
                std::cerr << "Expected ')'\n";
                std::exit(EXIT_FAILURE);
            }
            consume(); // ')'
            return expr;
        }

        return {};
    }

    // parses multiplication/division (left-associative, higher precedence than +/-)
    std::optional<NodeExpr *> parse_mul_expr()
    {
        auto left = parse_primary_expr();
        if (!left) return {};

        NodeExpr *current = left.value();

        while (peek().has_value() && (peek()->type == TokenType::mul || peek()->type == TokenType::div)) {
            TokenType op = consume().type; // consume '*' or '/'

            auto rhs_expr = parse_primary_expr();
            if (!rhs_expr) {
                std::cerr << "Expected term after '*' or '/'\n";
                std::exit(EXIT_FAILURE);
            }

            auto bin = m_allocator.alloc<NodeBinExpr>();
            if (op == TokenType::mul) {
                auto mul = m_allocator.alloc<NodeBinExprMulti>();
                mul->lhs = current;
                mul->rhs = rhs_expr.value();
                bin->var = mul;
            } else {
                auto div = m_allocator.alloc<NodeBinExprDiv>();
                div->lhs = current;
                div->rhs = rhs_expr.value();
                bin->var = div;
            }

            current = m_allocator.alloc<NodeExpr>();
            current->var = bin;
        }

        return current;
    }

    // top-level expression parser: handles + and - (left-associative) of mul-exprs
    std::optional<NodeExpr *> parse_expr()
    {
        auto left = parse_mul_expr();
        if (!left) return {};

        NodeExpr *current = left.value();

        while (peek().has_value() && (peek()->type == TokenType::plus || peek()->type == TokenType::minus)) {
            TokenType op = consume().type; // consume '+' or '-'

            auto right = parse_mul_expr(); 
            if (!right) {
                std::cerr << "Expected expression after '+' or '-'\n";
                std::exit(EXIT_FAILURE);
            }

            auto bin = m_allocator.alloc<NodeBinExpr>();
            if (op == TokenType::plus) {
                auto add = m_allocator.alloc<NodeBinExprAdd>();
                add->lhs = current;
                add->rhs = right.value();
                bin->var = add;
            } else {
                auto sub = m_allocator.alloc<NodeBinExprSub>();
                sub->lhs = current;
                sub->rhs = right.value();
                bin->var = sub;
            }

            current = m_allocator.alloc<NodeExpr>();
            current->var = bin;
        }

        return current;
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
                node_stmt_exit->expr = node_expr.value();

                auto node_stmt = m_allocator.alloc<NodeStmt>();
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
                node_stmt_let->ident = ident_token;
                node_stmt_let->expr = expr.value();

                auto node_stmt = m_allocator.alloc<NodeStmt>();
                node_stmt->var = node_stmt_let;
                return node_stmt;
            }
            // <ident> = expr;
            else if (peek()->type == TokenType::ident && peek(1).has_value() && peek(1)->type == TokenType::eq)
            {
                Token ident_token = consume(); // consume identifier
                consume();                     // consume '='
                auto expr = parse_expr();
                if (!expr)
                {
                    std::cerr << "Invalid expression in assignment statement\n";
                    std::exit(EXIT_FAILURE);
                }
                if (!peek().has_value() || peek()->type != TokenType::semi)
                {
                    std::cerr << "Expected ';' after assignment statement\n";
                    std::exit(EXIT_FAILURE);
                }
                consume(); // consume ';'

                auto node_stmt_assign = m_allocator.alloc<NodeStmtAssign>();
                node_stmt_assign->ident = ident_token;
                node_stmt_assign->expr = expr.value();

                auto node_stmt = m_allocator.alloc<NodeStmt>();
                node_stmt->var = node_stmt_assign;
                return node_stmt;
            }
            // { stmts* }
            else if (peek()->type == TokenType::open_curly)
            {
                consume(); // consume '{'
                auto node_stmt_block = m_allocator.alloc<NodeStmtBlock>();
                while (peek().has_value() && peek()->type != TokenType::close_curly)
                {
                    if (auto stmt = parse_stat())
                    {
                        node_stmt_block->stmts.push_back(stmt.value());
                    }
                    else
                    {
                        std::cerr << "Invalid statement inside block\n";
                        std::exit(EXIT_FAILURE);
                    }
                }
                if (!peek().has_value() || peek()->type != TokenType::close_curly)
                {
                    std::cerr << "Expected '}'\n";
                    std::exit(EXIT_FAILURE);
                }
                consume(); // consume '}'

                auto node_stmt = m_allocator.alloc<NodeStmt>();
                node_stmt->var = node_stmt_block;
                return node_stmt;
            }
            // if (expr) stmt [else stmt]
            else if (peek()->type == TokenType::if_tok)
            {
                consume(); // consume 'if'
                if (!peek().has_value() || peek()->type != TokenType::open_paren)
                {
                    std::cerr << "Expected '(' after if\n";
                    std::exit(EXIT_FAILURE);
                }
                consume(); // consume '('
                auto cond = parse_expr();
                if (!cond)
                {
                    std::cerr << "Expected condition expression inside if(...)\n";
                    std::exit(EXIT_FAILURE);
                }
                if (!peek().has_value() || peek()->type != TokenType::close_paren)
                {
                    std::cerr << "Expected ')' after if condition\n";
                    std::exit(EXIT_FAILURE);
                }
                consume(); // consume ')'
                
                auto then_stmt = parse_stat();
                if (!then_stmt)
                {
                    std::cerr << "Expected statement after if condition\n";
                    std::exit(EXIT_FAILURE);
                }

                std::optional<NodeStmt*> else_stmt = std::nullopt;
                if (peek().has_value() && peek()->type == TokenType::else_tok)
                {
                    consume(); // consume 'else'
                    auto else_stmt_node = parse_stat();
                    if (!else_stmt_node)
                    {
                        std::cerr << "Expected statement after else keyword\n";
                        std::exit(EXIT_FAILURE);
                    }
                    else_stmt = else_stmt_node.value();
                }

                auto node_stmt_if = m_allocator.alloc<NodeStmtIf>();
                node_stmt_if->cond = cond.value();
                node_stmt_if->then_stmt = then_stmt.value();
                node_stmt_if->else_stmt = else_stmt;

                auto node_stmt = m_allocator.alloc<NodeStmt>();
                node_stmt->var = node_stmt_if;
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
    ArenaAllocator m_allocator;
};