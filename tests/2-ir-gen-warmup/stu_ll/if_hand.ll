define dso_local i32 @main()#0{
    %1 = alloca float,align 4
    store float 0x40163851E0000000,float* %1,align 4
    %2 = load float, float* %1,align 4
    %3 = fcmp ogt float %2, 1.000000e+00
    %4 = alloca i32,align 4
    br i1 %3, label %5, label %7
5:   
    store i32 233, i32* %4,align 4
    br label %7
6:
    store i32 0, i32* %4,align 4   
    br label %7
7:
    %8 = load i32, i32* %4,align 4
    ret i32 %8
}