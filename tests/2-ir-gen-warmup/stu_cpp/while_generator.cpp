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

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto aAlloc = builder->create_alloca(Int32Type);
    auto iAlloc = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(10),aAlloc);
    builder->create_store(CONST_INT(0),iAlloc);
    auto detBB = BasicBlock::create(module, "detBB", mainFun);//detect 
    auto loopBB = BasicBlock::create(module, "loopBB", mainFun);//loop
    auto retBB = BasicBlock::create(module, "retBB", mainFun);//ret
    
    auto br = builder->create_br(detBB);
    builder->set_insert_point(detBB);
    auto iLoad = builder->create_load(iAlloc);
    auto icmp = builder->create_icmp_lt(iLoad,CONST_INT(10));
    builder->create_cond_br(icmp,loopBB,retBB);

    builder->set_insert_point(loopBB);
    auto iLoad_1 = builder->create_load(iAlloc);
    auto aLoad = builder->create_load(aAlloc);
    auto iadd = builder->create_iadd(iLoad_1,CONST_INT(1));
    auto aAddi = builder->create_iadd(aLoad,iadd);
    builder->create_store(iadd,iAlloc);
    builder->create_store(aAddi,aAlloc);
    builder->create_br(detBB);

    builder->set_insert_point(retBB);
    auto retNum = builder->create_load(aAlloc);
    builder->create_ret(retNum);

    std::cout << module->print();
    delete module;
    return 0;

}