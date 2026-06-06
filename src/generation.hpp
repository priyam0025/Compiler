#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "./parser.hpp"

class Generator {
    public:
        inline Generator(NodeProg prog)
            : m_prog(std::move(prog))
        {
            m_scopes.push_back({}); // global scope
        }

        void gen_term(const NodeTerm* term) {
            struct TermVisitor {
                Generator* gen;
                void operator()(const NodeTermIntLit* term_int_lit) const {
                    gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << '\n';
                    gen->push("rax");
                }
                void operator()(const NodeTermIdent* term_ident) const {
                    auto var_opt = gen->lookup(term_ident->ident.value.value());
                    if (!var_opt.has_value()) {
                        std::cerr << "Undeclared identifier: " << term_ident->ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    const auto& var = var_opt.value();
                    std::stringstream offset;
                    offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]";
                    gen->push(offset.str());
                }
            };
            TermVisitor visitor {.gen = this};
            std::visit(visitor, term->var);
        }

        // New helpers for NodeTermIntLit* and NodeTermIdent* since NodeExpr can hold those directly
        void gen_term_intlit(const NodeTermIntLit* term_int_lit) {
            m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << '\n';
            push("rax");
        }

        void gen_term_ident(const NodeTermIdent* term_ident) {
            auto var_opt = lookup(term_ident->ident.value.value());
            if (!var_opt.has_value()) {
                std::cerr << "Undeclared identifier: " << term_ident->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
            const auto& var = var_opt.value();
            std::stringstream offset;
            offset << "QWORD [rsp + " << (m_stack_size - var.stack_loc - 1) * 8 << "]\n";
            push(offset.str());
        }

        void gen_expr(const NodeExpr* expr) 
        {
            struct ExprVisitor {
                Generator* gen;
                void operator()(const NodeTermIntLit* t) const {
                    gen->gen_term_intlit(t);
                }
                void operator()(const NodeTermIdent* id) const {
                    gen->gen_term_ident(id);
                }
               
                void operator()(const NodeBinExpr* bin_expr) const {
                    // Determine which binary kind we have
                    if (auto p_add = std::get_if<NodeBinExprAdd*>(&bin_expr->var)) {
                        NodeBinExprAdd* add = *p_add;
                        // Evaluate lhs then rhs -> both push their results on the stack
                        gen->gen_expr(add->lhs);
                        gen->gen_expr(add->rhs);

                        // rhs is on top, then lhs below it. Pop rhs into rbx, lhs into rax
                        gen->pop("rbx");
                        gen->pop("rax");
                        gen->m_output << "    add rax, rbx\n";
                        gen->push("rax");
                    }
                    else if (auto p_sub = std::get_if<NodeBinExprSub*>(&bin_expr->var)) {
                        NodeBinExprSub* sub = *p_sub;
                        gen->gen_expr(sub->lhs);
                        gen->gen_expr(sub->rhs);

                        // lhs in rax, rhs in rbx, compute lhs - rhs
                        gen->pop("rbx");
                        gen->pop("rax");
                        gen->m_output << "    sub rax, rbx\n";
                        gen->push("rax");
                    }
                    else if (auto p_mul = std::get_if<NodeBinExprMulti*>(&bin_expr->var)) {
                        NodeBinExprMulti* mul = *p_mul;
                        gen->gen_expr(mul->lhs);
                        gen->gen_expr(mul->rhs);

                        gen->pop("rbx");
                        gen->pop("rax");
                        gen->m_output << "    imul rax, rbx\n";
                        gen->push("rax");
                    }
                    else if (auto p_div = std::get_if<NodeBinExprDiv*>(&bin_expr->var)) {
                        NodeBinExprDiv* div = *p_div;
                        gen->gen_expr(div->lhs);
                        gen->gen_expr(div->rhs);

                        // Signed division: (rdx:rax) / rbx -> quotient in rax
                        gen->pop("rbx");
                        gen->pop("rax");
                        gen->m_output << "    cqo\n";
                        gen->m_output << "    idiv rbx\n";
                        gen->push("rax");
                    }
                    else {
                        std::cerr << "Unknown binary expression kind\n";
                        std::exit(EXIT_FAILURE);
                    }
                }
            };

            ExprVisitor visitor {.gen = this};
            std::visit(visitor, expr->var);
        }

        void gen_stmt(const NodeStmt* stmt) 
        {
            struct StmtVisitor {
                Generator* gen;
                void operator()(const NodeStmtExit* stmt_exit) const
                {
                    gen->gen_expr(stmt_exit->expr);
                    gen->m_output << "    mov rax, 60\n";
                    gen->pop("rdi");
                    gen->m_output << "    syscall\n";
                }
                void operator()(const NodeStmtLet* stmt_let) const
                {
                    if (gen->m_scopes.back().count(stmt_let->ident.value.value())) {
                        std::cerr << "Variable already declared in this scope: " << stmt_let->ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    // Evaluate RHS first so the value is pushed on the stack
                    gen->gen_expr(stmt_let->expr);

                    // Record the variable location as the current top of stack in the inner-most scope
                    gen->m_scopes.back().insert({stmt_let->ident.value.value(), Var {.stack_loc = gen->m_stack_size - 1}});
                }
                void operator()(const NodeStmtAssign* stmt_assign) const
                {
                    // Evaluate RHS first so the value is pushed on the stack
                    gen->gen_expr(stmt_assign->expr);

                    // Check if variable is declared in any accessible scope
                    auto var_opt = gen->lookup(stmt_assign->ident.value.value());
                    if (!var_opt.has_value()) {
                        std::cerr << "Undeclared identifier: " << stmt_assign->ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    const auto& var = var_opt.value();

                    // Pop expression result into RAX
                    gen->pop("rax");

                    // Move result from RAX to variable stack offset
                    gen->m_output << "    mov QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "], rax\n";
                }
                void operator()(const NodeStmtBlock* stmt_block) const
                {
                    gen->m_scopes.push_back({});
                    size_t initial_stack_size = gen->m_stack_size;

                    for (const NodeStmt* stmt : stmt_block->stmts) {
                        gen->gen_stmt(stmt);
                    }

                    size_t vars_declared = gen->m_stack_size - initial_stack_size;
                    if (vars_declared > 0) {
                        gen->m_output << "    add rsp, " << vars_declared * 8 << "\n";
                        gen->m_stack_size -= vars_declared;
                    }

                    gen->m_scopes.pop_back();
                }
                void operator()(const NodeStmtIf* stmt_if) const
                {
                    gen->gen_expr(stmt_if->cond);
                    gen->pop("rax");

                    size_t label_id = gen->m_label_count++;
                    std::string else_label = ".L_else_" + std::to_string(label_id);
                    std::string end_label = ".L_end_" + std::to_string(label_id);

                    gen->m_output << "    test rax, rax\n";
                    if (stmt_if->else_stmt.has_value()) {
                        gen->m_output << "    jz " << else_label << "\n";
                    } else {
                        gen->m_output << "    jz " << end_label << "\n";
                    }

                    gen->gen_stmt(stmt_if->then_stmt);

                    if (stmt_if->else_stmt.has_value()) {
                        gen->m_output << "    jmp " << end_label << "\n";
                        gen->m_output << else_label << ":\n";
                        gen->gen_stmt(stmt_if->else_stmt.value());
                    }

                    gen->m_output << end_label << ":\n";
                }
            };
            
            StmtVisitor visitor {.gen = this}; // Create an instance of the visitor and pass current Generator object
            std::visit(visitor, stmt->var); // std::visit applies the correct operator() based on the variant type
        }

        std::string gen_prog() 
        {
            m_output << "global _start\n_start:\n";

            for (const NodeStmt* stmt : m_prog.stmts) {
                gen_stmt(stmt);
            }

            m_output << "    mov rax, 60\n";
            m_output << "    mov rdi, 0\n";
            m_output << "    syscall\n";
            return m_output.str();
        }
        
    private:

        void push(const std::string reg) {
            m_output << "    push " << reg << '\n';
            m_stack_size++;
        }

        void pop(const std::string reg) {
            m_output << "    pop " << reg << '\n';
            m_stack_size--;
        }

        struct Var {
            size_t stack_loc;
        };

        std::optional<Var> lookup(const std::string& name) const {
            for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
                if (it->count(name)) {
                    return it->at(name);
                }
            }
            return std::nullopt;
        }
        
        const NodeProg m_prog;
        std::stringstream m_output;
        size_t m_stack_size = 0;
        std::vector<std::unordered_map<std::string, Var>> m_scopes {};
        size_t m_label_count = 0;
};