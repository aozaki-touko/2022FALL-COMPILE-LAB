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
    auto module = new Module("cminus code");
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    //create array
    auto *arrayType = ArrayType::get(Int32Type, 10);
    //auto initializer = ConstantZero::get(Int32Type, module);
    auto array_addr = builder->create_alloca(arrayType);

    auto array_gep0 = builder->create_gep(array_addr,{CONST_INT(0),CONST_INT(0)});

    builder->create_store(CONST_INT(10),array_gep0);

    auto a_0allo=builder->create_alloca(Int32Type);

    //auto array_gep0 = builder->create_gep(array_addr,{CONST_INT(0)});

    auto a_0load=builder->create_load(array_gep0);

    auto timesa_0 = builder->create_imul(a_0load,CONST_INT(2));
    
    auto array_gep1 = builder->create_gep(array_addr,{CONST_INT(0),CONST_INT(1)});

    builder->create_store(timesa_0,array_gep1);

    builder->create_ret(timesa_0);
    std::cout<<module->print();
    delete module;
    return 0;
}