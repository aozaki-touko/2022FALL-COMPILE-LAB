define dso_local i32 @main(){
    %1 = alloca [10 x i32],align 16
    %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
    store i32 10 ,i32* %2,align 4
    %3 = load i32, i32* %2,align 4
    %4 = mul i32 %3, 2
    %5 = getelementptr [10 x i32], [10 x i32]* %1, i64 0, i64 1
    store i32 %4, i32* %5, align 4
    %6 = load i32, i32* %5,align 4
    ret i32 %6
}