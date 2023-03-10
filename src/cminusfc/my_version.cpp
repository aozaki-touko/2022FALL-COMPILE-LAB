/*
 * 澹版槑锛氭湰浠ｇ爜涓?2020 绉?涓浗绉戝ぇ缂栬瘧鍘熺悊锛堟潕璇氾級璇剧▼瀹為獙鍙傝€冨疄鐜般€?
 * 璇蜂笉瑕佷互浠讳綍鏂瑰紡锛屽皢鏈唬鐮佷笂浼犲埌鍙互鍏紑璁块棶鐨勭珯鐐规垨浠撳簱
 */

#include "cminusf_builder.hpp"

#include "logging.hpp"

#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())


// TODO: Global Variable Declarations 
// You can define global variables here
// to store state. You can expand these
// definitions if you need to.

// function that is being built
Function *cur_fun = nullptr;

Value* val=nullptr;//val need to communicate in all prog
bool in_fun_scope = false; //if in a fun true else false ,don't need to enter repeatly
bool right_hand = false;
bool left_hand = false;
Value* temp_arg;
// types
Type *VOID_T;
Type *INT1_T;
Type *INT32_T;
Type *INT32PTR_T;
Type *FLOAT_T;
Type *FLOATPTR_T;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node) {
    VOID_T = Type::get_void_type(module.get());
    INT1_T = Type::get_int1_type(module.get());
    INT32_T = Type::get_int32_type(module.get());
    INT32PTR_T = Type::get_int32_ptr_type(module.get());
    FLOAT_T = Type::get_float_type(module.get());
    FLOATPTR_T = Type::get_float_ptr_type(module.get());

    for (auto decl : node.declarations) {
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    
    if(node.type==TYPE_INT)
    {
        val = CONST_INT(node.i_val);
    }
    else
    {
        val = CONST_FP(node.f_val);
    }

}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    Type *var_type ;
    if(node.type==TYPE_INT)
    {
        var_type = Type::get_int32_type(module.get());
    }
    else
    {
        var_type = Type::get_float_type(module.get());
    }
    
    if(node.num==nullptr)
    {
        //just an id
        if(scope.in_global())
        {
            //static GlobalVariable *create(std::string name, Module *m, Type *ty, bool is_const, Constant *init);
            //a global var
            
            auto initilizer = ConstantZero::get(var_type,module.get());
            auto var=GlobalVariable::create(node.id,module.get(),var_type,false,initilizer);
            scope.push(node.id,var);
        }
        else
        {
            //a local var
            auto var = builder->create_alloca(var_type);
            scope.push(node.id,var);
        }
    }
    else
    {
        //an array
        assert(node.num>0);
        auto arraytype=ArrayType::get_array_type(var_type,node.num->i_val);
        auto arrayInitilizer = ConstantZero::get(arraytype,module.get());
        if(scope.in_global())
        {
            
            auto initilizer = ConstantZero::get(var_type,module.get());
            auto var=GlobalVariable::create(node.id,module.get(),arraytype,false,arrayInitilizer);
            scope.push(node.id,var);
        }
        else
        {
            auto var = builder->create_alloca(arraytype);
            scope.push(node.id,var);
        }
    }


}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    FunctionType *fun_type;
    Type *ret_type;
    std::vector<Type *> param_types;
    if (node.type == TYPE_INT)
        ret_type = INT32_T;
    else if (node.type == TYPE_FLOAT)
        ret_type = FLOAT_T;
    else
        ret_type = VOID_T;

    for (auto &param : node.params) {
        //!TODO: Please accomplish param_types.

        if(param->isarray)
        {
            //deal with array
            if(param->type == TYPE_INT)
            {
                param_types.push_back(INT32PTR_T);//int array ptr
            }
            else
            {
                param_types.push_back(FLOATPTR_T);//float array ptr
            }
        }
        else
        {
            //deal with just a num
            if(param->type == TYPE_INT)
            {
                param_types.push_back(INT32_T);
            }
            else
            {
                param_types.push_back(FLOAT_T);
            }
        }
    }

    fun_type = FunctionType::get(ret_type, param_types);
    auto fun = Function::create(fun_type, node.id, module.get());
    scope.push(node.id, fun);
    cur_fun = fun;
    auto funBB = BasicBlock::create(module.get(), "entry", fun);
    builder->set_insert_point(funBB);
    scope.enter();
    std::vector<Value *> args;
    for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) {
        args.push_back(*arg);
    }
    for (int i = 0; i < node.params.size(); ++i) {
        //!TODO: You need to deal with params
        // and store them in the scope.
        
        temp_arg=args[i];
        node.params[i]->accept(*this);
    }
    //here we enter fun scope
    in_fun_scope = true;

    node.compound_stmt->accept(*this);
    if (builder->get_insert_block()->get_terminator() == nullptr) 
    {
        if (cur_fun->get_return_type()->is_void_type())
            builder->create_void_ret();
        else if (cur_fun->get_return_type()->is_float_type())
            builder->create_ret(CONST_FP(0.));
        else
            builder->create_ret(CONST_INT(0));
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node) {
    //!TODO: This function is empty now.
    // Add some code here.
    if(node.isarray)
        {
            //store 
            if(node.type==TYPE_INT)
            {
                Value* alloc = builder->create_alloca(INT32PTR_T);
                builder->create_store(temp_arg,alloc);
                //push which?
                scope.push(node.id,alloc);//store the addr because addr is the param identifier
            }
            else
            {
                Value* alloc = builder->create_alloca(FLOATPTR_T);
                builder->create_store(temp_arg,alloc);
                //push which?
                scope.push(node.id,alloc);
            }
        }
        else
        {
            Value* alloc;
            if(node.type==TYPE_INT)
            {
                alloc = builder->create_alloca(INT32_T);
                builder->create_store(temp_arg,alloc);
                scope.push(node.id,alloc);
            }
            else
            {
                alloc = builder->create_alloca(FLOAT_T);
                builder->create_store(temp_arg,alloc);
                scope.push(node.id,alloc);
            }
        }
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    //!TODO: This function is not complete.
    // You may need to add some code here
    // to deal with complex statements.

    //maybe declare the same name var
    bool non_fun_scope = !in_fun_scope;
    if(in_fun_scope)
    {
        in_fun_scope = false;
    }
    else
    {
        //we need to enter a new scope
        scope.enter();
    }
    // to deal with complex statements. 

    for (auto &decl : node.local_declarations) {
        decl->accept(*this);
    }

    for (auto &stmt : node.statement_list) {
        stmt->accept(*this);
        if (builder->get_insert_block()->get_terminator() != nullptr)
            break;
    }


    if(non_fun_scope)
    {
        scope.exit();
    }
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    if(node.expression!=nullptr)
    {
        node.expression->accept(*this);
    }

}

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    
    node.expression->accept(*this);
    auto tmp_val = val;
    //static BasicBlock *create(Module *m, const std::string &name, Function *parent)
    
    auto true_bb = BasicBlock::create(module.get(),"",cur_fun);
    auto false_bb = BasicBlock::create(module.get(),"",cur_fun);
    auto out_bb = BasicBlock::create(module.get(),"",cur_fun);
    Value *det;
    //now create det branch
    if(tmp_val->get_type()->is_integer_type())
    {
        det = builder->create_icmp_ne(tmp_val,CONST_INT(0));
    }
    else
    {
        det = builder->create_fcmp_ne(tmp_val,CONST_FP(0.0));
    }

    if(node.else_statement==nullptr)
    {
        //style:if A then B
        /*
        A:
            det
        true bb:
            ...
        */
       builder->create_cond_br(det,true_bb,out_bb);
    }
    else
    {
        //style:if A then B else C
        builder->create_cond_br(det,true_bb,false_bb);
    }

    //get true bb
    builder->set_insert_point(true_bb);
    node.if_statement->accept(*this);

    if(builder->get_insert_block()->get_terminator()==nullptr)// if not the last instr
    {
        //jump out
        builder->create_br(out_bb);
    }

    if(node.else_statement==nullptr)
    {
        false_bb->erase_from_parent();
    }
    else
    {
        builder->set_insert_point(false_bb);
        node.else_statement->accept(*this);
        if(builder->get_insert_block()->get_terminator()==nullptr)
        {
            builder->create_br(out_bb);
        }   
    }

    builder->set_insert_point(out_bb);



}

void CminusfBuilder::visit(ASTIterationStmt &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    /*
    style
    det:
        A
        true: iter
        false :out
    ITER:
        B
        goback det
    out:    
    */
    auto detbb = BasicBlock::create(module.get(),"",cur_fun);
    if(builder->get_insert_block()->get_terminator()==nullptr)
    {
        /*
        while()
        return;
        */
        builder->create_br(detbb);
    }
    builder->set_insert_point(detbb);
    node.expression->accept(*this);
    
    auto tmp_val = val;
    auto iterBB = BasicBlock::create(module.get(),"",cur_fun);
    auto outBB = BasicBlock::create(module.get(),"",cur_fun);
    Value *det;
    if(tmp_val->get_type()->is_integer_type())
    {
        det = builder->create_icmp_ne(tmp_val,CONST_INT(0));
    }
    else
    {
        det = builder->create_fcmp_ne(tmp_val,CONST_FP(0.0));
    }

    builder->create_cond_br(det,iterBB,outBB);
    builder->set_insert_point(iterBB);
    node.statement->accept(*this);
    
    if(builder->get_insert_block()->get_terminator()==nullptr)
    {
        builder->create_br(detbb);
    }
    builder->set_insert_point(outBB);


}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    if (node.expression == nullptr) {
        builder->create_void_ret();
    } else {
        //!TODO: The given code is incomplete.
        // You need to solve other return cases (e.g. return an integer).

        auto returnType = cur_fun->get_return_type();
        node.expression->accept(*this);
        if(val->get_type()!=returnType)
        {
            //need to convert
            if(returnType->is_integer_type())
            {
                //float -> int
                val = builder->create_fptosi(val,INT32_T);
            }
            else
            {
                //int -> float
                val = builder->create_sitofp(val,FLOAT_T);
            }
        }
        builder->create_ret(val);



    }
}

void CminusfBuilder::visit(ASTVar &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    auto exist = scope.find(node.id);
    if(exist==nullptr)
    {
        std::cerr<<node.id<<":no exist"<<std::endl;
    }

    auto current_place = right_hand;
    right_hand = false;

    if(node.expression==nullptr)
    {
        //just an ID
        if(current_place == 1)
        {
            val = exist;
            right_hand = false;
        }
        else
        {
            //right hand size ,just load it
            if(exist->get_type()->get_pointer_element_type()->is_integer_type()||exist->get_type()->get_pointer_element_type()->is_float_type()||exist->get_type()->get_pointer_element_type()->is_pointer_type())
            {
                val = builder->create_load(exist);
            }
            else
            {
                //an array ,which is a pointer to pointer
                val = builder->create_gep(exist,{CONST_INT(0),CONST_INT(0)});
                
            }
        }
    }
    else
    {
        //array
        node.expression->accept(*this);
        Value* array_addr = val;
        //calc the addr
        auto neg_bb = BasicBlock::create(module.get(),"",cur_fun);
        auto legal_bb = BasicBlock::create(module.get(),"",cur_fun);
        if(array_addr->get_type()!=INT32_T)
        {
            array_addr=builder->create_fptosi(array_addr,INT32_T);
        }
        Value* isillegal = builder->create_icmp_lt(array_addr,CONST_INT(0));

        builder->create_cond_br(isillegal,neg_bb,legal_bb);

        builder->set_insert_point(neg_bb);
        //call
        builder->create_call(static_cast<Function *>(scope.find("neg_idx_except")),{});
        if(cur_fun->get_return_type()->is_integer_type())
        {
            builder->create_ret(CONST_INT(0));
        }
        else if(cur_fun->get_return_type()->is_float_type())
        {
            builder->create_ret(CONST_FP(0.0));
        }
        else
        {
            builder->create_void_ret();
        }
        
        //exit the function
        builder->set_insert_point(legal_bb);
        Value *temp_ptr;
        //we all change to a pointer
        if(exist->get_type()->get_pointer_element_type()->is_integer_type()||exist->get_type()->get_pointer_element_type()->is_float_type())
        {
            temp_ptr = builder->create_gep(exist,{array_addr});
        }
        else if(exist->get_type()->get_pointer_element_type()->is_pointer_type())
        {
            auto load = builder->create_load(exist);
            temp_ptr = builder->create_gep(load,{array_addr});
        }
        else
        {
            //pointer to pointer
            temp_ptr = builder->create_gep(exist,{CONST_INT(0),array_addr});
        }
        if(current_place==1)
        {
            val = temp_ptr;
            right_hand = 0;
        }
        else
        {
            val = builder->create_load(temp_ptr);
        }

    }


}

void CminusfBuilder::visit(ASTAssignExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.


    
    node.expression->accept(*this);
    auto expr_val = val;
    right_hand = true;//represent tmp_val is right value;
    //left_hand = false;
    node.var->accept(*this);
    auto var_ptr = val;
    if(var_ptr->get_type()->get_pointer_element_type()==expr_val->get_type())
    {
        //var_ptr(value*) is a pointer, need to trans to type* then get elem type
        //dont need to convert
        builder->create_store(expr_val,var_ptr);
        val = expr_val;
    }
    else
    {
        //type is not same,need to convert

        if(var_ptr->get_type()->get_pointer_element_type()==INT32_T)
        {
            expr_val = builder->create_fptosi(expr_val,INT32_T);
        }
        else
        {
            expr_val = builder->create_sitofp(expr_val,FLOAT_T);
        }
        builder->create_store(expr_val,var_ptr);
        val = expr_val;
    }
    


}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    if(node.additive_expression_r==nullptr)
    {
        //just left 
        node.additive_expression_l->accept(*this);
    }
    else
    {
        //same as calc ast
        node.additive_expression_l->accept(*this);
        auto lval = val;
        node.additive_expression_r->accept(*this);
        auto rval = val;
        bool flag = true;
        //check and fix type
        if(lval->get_type()==rval->get_type())
        {
            flag = lval->get_type()->is_integer_type();
        }
        else
        {
            flag = false;
            if(lval->get_type()->is_integer_type())
            {
                //lval is int and rval is float
                lval=builder->create_sitofp(lval,FLOAT_T);
                
            }
            else if(rval->get_type()->is_integer_type())
            {
                rval = builder->create_sitofp(rval,FLOAT_T);
            }
        }


        //create cmp
        Value* cmp_res;
        if(flag)
        {
            
            //int relop
            switch (node.op)
            {
            case OP_LE:
                cmp_res=builder->create_icmp_le(lval,rval);
                break;
            case OP_LT:
                cmp_res=builder->create_icmp_lt(lval,rval);
                break;
            case OP_GT:
                cmp_res=builder->create_icmp_gt(lval,rval);
                break;
            case OP_GE:
                cmp_res=builder->create_icmp_ge(lval,rval);
                break;
            case OP_EQ:
                cmp_res=builder->create_icmp_eq(lval,rval);
                break;
            case OP_NEQ:
                cmp_res=builder->create_icmp_ne(lval,rval);
                break;
            default:
                break;
            }
        }
        else
        {
            //float relop
            switch (node.op)
            {
            case OP_LE:
                cmp_res=builder->create_fcmp_le(lval,rval);
                break;
            case OP_LT:
                cmp_res=builder->create_fcmp_lt(lval,rval);
                break;
            case OP_GT:
                cmp_res=builder->create_fcmp_gt(lval,rval);
                break;
            case OP_GE:
                cmp_res=builder->create_fcmp_ge(lval,rval);
                break;
            case OP_EQ:
                cmp_res=builder->create_fcmp_eq(lval,rval);
                break;
            case OP_NEQ:
                cmp_res=builder->create_fcmp_ne(lval,rval);
                break;
            default:
                break;
            }
        }
        //do a zext to make type same
        val = builder->create_zext(cmp_res,INT32_T);
    }

}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    if(node.additive_expression==nullptr)
    {
        node.term->accept(*this);

    }
    else
    {
        node.additive_expression->accept(*this);
        auto lval = val;
        node.term->accept(*this);
        auto rval = val;


        bool flag = true;
        //check and fix type
        if(lval->get_type()==rval->get_type())
        {
            flag = lval->get_type()->is_integer_type();
        }
        else
        {
            flag = false;
            if(lval->get_type()->is_integer_type())
            {
                //lval is int and rval is float
                lval=builder->create_sitofp(lval,FLOAT_T);
                
            }
            else if(rval->get_type()->is_integer_type())
            {
                rval = builder->create_sitofp(rval,FLOAT_T);
            }
        }


        //create add op
        Value* additive_val;
        if(flag)
        {
            //int type
            if(node.op==OP_PLUS)
            {
                additive_val = builder->create_iadd(lval,rval); 
            }
            else
            {
                additive_val = builder->create_isub(lval,rval);
            }
        }
        else
        {
            //float type
            if(node.op==OP_PLUS)
            {
                additive_val = builder->create_fadd(lval,rval);
            }
            else
            {
                additive_val = builder->create_fsub(lval,rval);
            }
            
        }
        val = additive_val;
    }

}

void CminusfBuilder::visit(ASTTerm &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    if(node.term==nullptr)
    {
        node.factor->accept(*this);
    }
    else
    {
        node.term->accept(*this);
        auto lval = val;
        node.factor->accept(*this);
        auto rval = val;


        bool flag = true;
        //check and fix type
        if(lval->get_type()==rval->get_type())
        {
            flag = lval->get_type()->is_integer_type();
        }
        else
        {
            flag = false;
            if(lval->get_type()->is_integer_type())
            {
                //lval is int and rval is float
                lval=builder->create_sitofp(lval,FLOAT_T);
                
            }
            else if(rval->get_type()->is_integer_type())
            {
                rval = builder->create_sitofp(rval,FLOAT_T);
            }
        }

        //create mul/div op
        Value *calc_res;
        if(flag)
        {
            //int type 
            if(node.op == OP_MUL)
            {
                calc_res = builder->create_imul(lval,rval);
            }
            else
            {
                calc_res = builder->create_isdiv(lval,rval);
            }
        }
        else
        {
            //float type
            if(node.op == OP_MUL)
            {
                calc_res = builder->create_fmul(lval,rval);
            }
            else
            {
                calc_res = builder->create_fdiv(lval,rval);
            }
            
        }
        val =calc_res;
    }

}

void CminusfBuilder::visit(ASTCall &node) {
    //!TODO: This function is empty now.
    // Add some code here.

    auto fun = static_cast<Function *>(scope.find(node.id));
    std::vector<Value *>args;
    int count =0;
    for(auto &i:node.args)
    {
        auto ptr_param = fun->get_function_type()->get_param_type(count);
        i->accept(*this);
        if(val->get_type()!=(ptr_param)&&(val->get_type()->is_pointer_type()==false))
        {
            if(val->get_type()->is_integer_type())
            {
                val = builder->create_sitofp(val,FLOAT_T);
            }
            else
            {
                val = builder->create_fptosi(val,INT32_T);
            }
        }
        args.push_back(val);
        ptr_param++;
        count++;
    }
    val = builder->create_call(static_cast<Function *>(fun),args);
    
}
