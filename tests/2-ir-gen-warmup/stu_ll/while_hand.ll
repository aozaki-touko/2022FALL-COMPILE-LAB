define dso_local i32 @main() #0 {
    %1 = alloca i32,align 4 ;a addr
    %2 = alloca i32,align 4 ;i addr
    store i32 10, i32* %1,align 4
    store i32 0, i32* %2,align 4
    %3 = load i32, i32* %1,align 4 ;a  
    br label %4
4:
    %5 = load i32, i32* %2,align 4 ;i
    %6 = icmp slt i32 %5, 10
    br i1 %6, label %7, label %12
7:
    %8 = load i32, i32* %1,align 4 ;a
    %9 = load i32, i32* %2,align 4 ;i
    %10 = add i32 %9, 1 ;i+1
    %11 = add i32 %8, %10 ;a+i
    store i32 %11, i32* %1,align 4
    store i32 %10, i32* %2,align 4
    br label %4
12:
    %13 = load i32, i32* %1,align 4 ;i
    ret i32 %13
}