target datalayout = "@BACKEND_TARGET_DATALAYOUT@"
target triple = "@BACKEND_TARGET_TRIPLE@"

%blockheader = type { i64 } 
%block = type { %blockheader, [0 x ptr] } ; 16-bit layout, 8-bit length, 32-bit tag, children

declare ptr @parse_configuration(ptr)
declare i64 @atol(ptr)
declare ptr @fopen(ptr, ptr)

declare ptr @take_steps(i64, ptr)
declare void @finish_rewriting(ptr, i1) #0

declare void @init_static_objects()

declare void @print_proof_hint_header(ptr)

@statistics.flag = private constant [13 x i8] c"--statistics\00"
@binary_out.flag = private constant [16 x i8] c"--binary-output\00"
@proof_out.flag = private constant [15 x i8] c"--proof-output\00"

@output_file = external global ptr
@a_str = private constant [2 x i8] c"a\00"
@statistics = external global i1
@binary_output = external global i1
@proof_output = external global i1

declare i32 @strcmp(ptr %a, ptr %b)

define void @parse_flags(i32 %argc, ptr %argv) {
entry:
  store i1 0, ptr @statistics
  br label %header

header:
  %idx = phi i32 [ 4, %entry ], [ %idx.inc, %inc ]
  %continue = icmp slt i32 %idx, %argc
  br i1 %continue, label %body, label %exit

body:
  %argv.idx = getelementptr inbounds ptr, ptr %argv, i32 %idx
  %arg = load ptr, ptr %argv.idx
  br label %body.stats

body.stats:
  %stats.cmp = call i32 @strcmp(ptr %arg, ptr getelementptr inbounds ([13 x i8], ptr @statistics.flag, i64 0, i64 0))
  %stats.eq = icmp eq i32 %stats.cmp, 0
  br i1 %stats.eq, label %set.stats, label %binary.body

set.stats:
  store i1 1, ptr @statistics
  br label %binary.body

binary.body:
  %binary.cmp = call i32 @strcmp(ptr %arg, ptr getelementptr inbounds ([16 x i8], ptr @binary_out.flag, i64 0, i64 0))
  %binary.eq = icmp eq i32 %binary.cmp, 0
  br i1 %binary.eq, label %binary.set, label %proof.body

binary.set:
  store i1 1, ptr @binary_output
  br label %proof.body

proof.body:
  %proof.cmp = call i32 @strcmp(ptr %arg, ptr getelementptr inbounds ([15 x i8], ptr @proof_out.flag, i64 0, i64 0))
  %proof.eq = icmp eq i32 %proof.cmp, 0
  br i1 %proof.eq, label %proof.set, label %body.tail

proof.set:
  store i1 1, ptr @proof_output
  br label %body.tail

body.tail:
  br label %inc

inc:
  %idx.inc = add i32 %idx, 1
  br label %header

exit:
  ret void
}

define i32 @main(i32 %argc, ptr %argv) {
entry:
  %filename_ptr = getelementptr inbounds ptr, ptr %argv, i64 1
  %filename = load ptr, ptr %filename_ptr
  %depth_ptr = getelementptr inbounds ptr, ptr %argv, i64 2
  %depth_str = load ptr, ptr %depth_ptr
  %depth = call i64 @atol(ptr %depth_str)
  %output_ptr = getelementptr inbounds ptr, ptr %argv, i64 3
  %output_str = load ptr, ptr %output_ptr
  %output_file = call ptr @fopen(ptr %output_str, ptr getelementptr inbounds ([2 x i8], ptr @a_str, i64 0, i64 0))
  store ptr %output_file, ptr @output_file
  
  call void @parse_flags(i32 %argc, ptr %argv)

  call void @init_static_objects()

  %proof_output = load i1, ptr @proof_output
  br i1 %proof_output, label %if, label %else
if:
  call void @print_proof_hint_header(ptr %output_file)
  br label %else
else:
  %ret = call ptr @parse_configuration(ptr %filename)
  %result = call ptr @take_steps(i64 %depth, ptr %ret)
  call void @finish_rewriting(ptr %result, i1 0)
  unreachable
}

attributes #0 = { noreturn }
