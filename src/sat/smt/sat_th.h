/*++
Copyright (c) 2020 Microsoft Corporation

Module Name:

    sat_th.h

Abstract:

    Theory plugins

Author:

    Nikolaj Bjorner (nbjorner) 2020-08-25

--*/
#pragma once

#include "util/top_sort.h"
#include "sat/smt/sat_smt.h"
#include "ast/euf/euf_egraph.h"

namespace euf {

    class solver;
    
    class th_internalizer {
    public:
        virtual ~th_internalizer() {}

        virtual sat::literal internalize(expr* e, bool sign, bool root, bool redundant) = 0;
    };

    class th_decompile {
    public:
        virtual ~th_decompile() {}

        virtual bool to_formulas(std::function<expr_ref(sat::literal)>& lit2expr, expr_ref_vector& fmls) = 0;
    };

    class th_model_builder {
    public:

        virtual ~th_model_builder() {}

        /**
           \brief compute the value for enode \c n and store the value in \c values 
           for the root of the class of \c n.
         */
        virtual void add_value(euf::enode* n, expr_ref_vector& values) {}

        /**
           \brief compute dependencies for node n
         */
        virtual void add_dep(euf::enode* n, top_sort<euf::enode>& dep) {}

        /**
           \brief should function be included in model.
        */
        virtual bool include_func_interp(func_decl* f) const { return false; }
    };

    class th_solver : public sat::extension, public th_model_builder, public th_decompile, public th_internalizer {
    protected:
        ast_manager &       m;
        euf::theory_id      m_id;
    public:
        th_solver(ast_manager& m, euf::theory_id id): m(m), m_id(id) {}

        virtual th_solver* fresh(sat::solver* s, euf::solver& ctx) = 0;  

        virtual void new_eq_eh(euf::th_eq const& eq) {}
    };

    class th_euf_solver : public th_solver {
    protected:
        solver &            ctx;
        euf::enode_vector   m_var2enode;
        unsigned_vector     m_var2enode_lim;
    public:
        virtual ~th_euf_solver() {}

        th_euf_solver(euf::solver& ctx, euf::theory_id id);           

        virtual euf::theory_var mk_var(enode * n) {
            SASSERT(!is_attached_to_var(n));
            euf::theory_var v = m_var2enode.size();
            m_var2enode.push_back(n);
            return v;
        }

        enode* get_enode(theory_var v) const { return m_var2enode[v]; }

        euf::theory_var get_th_var(expr* e) const;

        euf::theory_var get_th_var(euf::enode* n) const {
            return n->get_th_var(get_id());
        }

        bool is_attached_to_var(enode* n) const {
            theory_var v = n->get_th_var(get_id());
            return v != null_theory_var && get_enode(v) == n;
        }

    };


}