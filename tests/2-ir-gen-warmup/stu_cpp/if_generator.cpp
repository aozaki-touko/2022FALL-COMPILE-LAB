#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl; // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) ConstantInt::get(num, module)

#define CONST_FP(num) ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到
int main()
{
    auto module = new Module("Cminus code");  // module name是什么无关紧要
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    Type *FloatTy = Type::get_float_type(module);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    auto iAlloc = builder->create_alloca(FloatTy);
    auto retAlloc = builder->create_alloca(Int32Type);
    builder->create_store(CONST_FP(5.555),iAlloc);
    auto iLoad = builder->create_load(iAlloc);
    auto fcmp = builder->create_fcmp_gt(iLoad,CONST_FP(1.00));

    auto trueBB = BasicBlock::create(module,"truebb",mainFun);
    auto falseBB = BasicBlock::create(module,"falsebb",mainFun);
    auto retBB = BasicBlock::create(module, "", mainFun);

    auto br = builder->create_cond_br(fcmp,trueBB,falseBB);

    builder->set_insert_point(trueBB);  // if true; s分支的开始需要SetInsertPoint设置
    builder->create_store(CONST_INT(233),retAlloc);
    builder->create_br(retBB);

    builder->set_insert_point(falseBB);
    builder->create_store(CONST_INT(0),retAlloc);
    builder->create_br(retBB);

    builder->set_insert_point(retBB);
    auto retNum = builder->create_load(retAlloc);
    builder->create_ret(retNum);

    std::cout << module->print();
    delete module;
    return 0;

}