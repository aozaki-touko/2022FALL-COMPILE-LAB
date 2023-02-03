; ModuleID = 'cminus'
source_filename = "/labs/2022fall-compiler_cminus/tests/4-ir-opt/testcases/GVN/functional/single_bb1.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @main() {
label_entry:
  %op0 = call i32 @input()
  %op1 = call i32 @input()
  %op2 = add i32 %op0, %op1
  call void @output(i32 %op2)
  %op3 = add i32 %op0, %op1
  call void @output(i32 %op3)
  ret i32 0
}
