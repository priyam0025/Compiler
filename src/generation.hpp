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
        }

        void gen_term(const NodeTerm* term) {
            struct TermVisitor {
                Generator* gen;
                void operator()(const NodeTermIntLit* term_int_lit) const {
                    gen->m_output << "    mov rax, " << term_int_lit->int_lit.value.value() << '\n';
                    gen->push("rax");
                }
                void operator()(const NodeTermIdent* term_ident) const {
                    if (!gen->m_vars.count(term_ident->ident.value.value())) {
                        std::cerr << "Undeclaired identifier : " << term_ident->ident.value.value() << std::endl;
                        exit(EXIT_FAILURE);
                    }
                    const auto& var = gen->m_vars.at(term_ident->ident.value.value());
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
            if (!m_vars.count(term_ident->ident.value.value())) {
                std::cerr << "Undeclaired identifier : " << term_ident->ident.value.value() << std::endl;
                exit(EXIT_FAILURE);
            }
            const auto& var = m_vars.at(term_ident->ident.value.value());
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
                    // Evaluate RHS first so the value is pushed on the stack
                    gen->gen_expr(stmt_let->expr);

                    // Record the variable location as the current top of stack
                    gen->m_vars.insert({stmt_let->ident.value.value(), Var {.stack_loc = gen->m_stack_size - 1}});
                }
            };
            
            StmtVisitor visitor {.gen = this};
            std::visit(visitor, stmt->var);
        }

        std::string gen_prog() 
        {
            // use the member output stream (non-const method)
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
        
        const NodeProg m_prog;
        std::stringstream m_output;
        size_t m_stack_size = 0;
        std::unordered_map<std::string, Var> m_vars {};
};