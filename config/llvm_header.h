R"LLVM(
target datalayout = "@BACKEND_TARGET_DATALAYOUT@"
target triple = "@BACKEND_TARGET_TRIPLE@"

; K types in LLVM

; A K value in the LLVM backend can be one of the following values:

; an uninterpreted domain value: \dv{Id}("foo")
; a symbol with 0 or more arguments: foo{}()
; a map: dotMap{}()
; an associative list: dotList{}()
; a set: dotSet{}()
; an array: dotArray{}()
; an arbitrary precision integer: \dv{Int}("0")
; an arbitrary precision float: \dv{Float}("0.0")
; a domain string: \dv{String}("\"\"")
; a byte array: \dv{Bytes}("b\"\"")
; a string buffer: \dv{StringBuffer}("")
; a domain boolean: \dv{Bool}("false")
; a machine integer: \dv{MInt}("0p8")

; For each type, a value of that type has the following llvm type:

; token: %string *
; symbol with 0 arguments: i32
; symbol with 1 or more arguments: %block *
; map: %map
; list: %list
; set: %set
; array: %list
; integer: %mpz *
; float: %floating *
; string: %string *
; bytes: %string *
; string buffer: %stringbuffer *
; boolean: i1
; machine integer of N bits: iN

; We also define the following LLVM structure types:

%string = type { %blockheader, [0 x i8] } ; 10-bit layout, 4-bit gc flags, 10 unused bits, 40-bit length (or buffer capacity for string pointed by stringbuffers), bytes
%stringbuffer = type { i64, i64, %string* } ; 10-bit layout, 4-bit gc flags, 10 unused bits, 40-bit length, string length, current contents
%map = type { { i8 *, i64 } } ; immer::map
%rangemap = type { { { { { i32 (...)**, i32, i64 }*, { { i32 (...)**, i32, i32 }* } } } } } ; rng_map::RangeMap
%set = type { { i8 *, i64 } } ; immer::set
%iter = type { { i8 *, i8 *, i32, [14 x i8**] }, { { i8 *, i64 } } } ; immer::map_iter / immer::set_iter
%list = type { { i64, i32, i8 *, i8 * } } ; immer::flex_vector
%mpz = type { i32, i32, i64 * } ; mpz_t
%mpz_hdr = type { %blockheader, %mpz } ; 10-bit layout, 4-bit gc flags, 10 unused bits, 40-bit length, mpz_t
%floating = type { i64, { i64, i32, i64, i64 * } } ; exp, mpfr_t
%floating_hdr = type { %blockheader, %floating } ; 10-bit layout, 4-bit gc flags, 10 unused bits, 40-bit length, floating
%blockheader = type { i64 }
%block = type { %blockheader, [0 x i64 *] } ; 16-bit layout, 8-bit length, 32-bit tag, children

%layout = type { i8, %layoutitem* } ; number of children, array of children
%layoutitem = type { i64, i16 } ; offset, category

; The layout of a block uniquely identifies the categories of its children as
; well as how to allocate/deallocate them and whether to follow their pointers
; during gc. Roughly speaking, the following rules apply:

; %string *: malloc/free, do not follow
; iN: noop, do not follow
; %map, %set, %list: noop/drop_in_place, follow
; %block *: managed heap, follow
; %mpz *: malloc/mpz_clear->free, do not follow
; %floating *: malloc/mpfr_clear->free, do not follow
; %stringbuffer *: malloc->malloc/free->free, do not follow

; We also automatically generate for each unique layout id a struct type
; corresponding to the actual layout of that block. For example, if we have
; the symbol symbol foo{Map{}, Int{}, Exp{}} : Exp{}, we would generate the type:

; %layoutN = type { %blockheader, [0 x i64 *], %map, %mpz *, %block * }

; Interface to the configuration parser
declare %block* @parse_configuration(i8*)
declare void @print_configuration(i8 *, %block *)

; get_exit_code

declare i64 @__gmpz_get_ui(%mpz*)

@exit_int_0 = global %mpz { i32 0, i32 0, i64* getelementptr inbounds ([0 x i64], [0 x i64]* @exit_int_0_limbs, i32 0, i32 0) }
@exit_int_0_limbs = global [0 x i64] zeroinitializer

define tailcc %mpz* @"eval_LblgetExitCode{SortGeneratedTopCell{}}"(%block*) {
  ret %mpz* @exit_int_0
}

define i32 @get_exit_code(%block* %subject) {
  %exit_z = call tailcc %mpz* @"eval_LblgetExitCode{SortGeneratedTopCell{}}"(%block* %subject)
  %exit_ul = call i64 @__gmpz_get_ui(%mpz* %exit_z)
  %exit_trunc = trunc i64 %exit_ul to i32
  ret i32 %exit_trunc
}

; get_fresh_constant

declare void @abort() #0

declare i32 @get_tag_for_fresh_sort(i8*)
declare %mpz* @hook_INT_add(%mpz*, %mpz*)
declare i8* @evaluate_function_symbol(i32, i8**)
declare i8* @get_terminated_string(%string*)

@fresh_int_1 = global %mpz { i32 1, i32 1, i64* getelementptr inbounds ([1 x i64], [1 x i64]* @fresh_int_1_limbs, i32 0, i32 0) }
@fresh_int_1_limbs = global [1 x i64] [i64 1]

define tailcc %block* @"eval_LblgetGeneratedCounterCell{SortGeneratedTopCell{}}"(%block*) {
  call void @abort()
  unreachable
}

define i8* @get_fresh_constant(%string* %sort, %block* %top) {
entry:
  %counterCell = call tailcc %block* @"eval_LblgetGeneratedCounterCell{SortGeneratedTopCell{}}"(%block* %top)
  %counterCellPointer = getelementptr %block, %block* %counterCell, i64 0, i32 1, i64 0
  %mpzPtrPtr = bitcast i64** %counterCellPointer to %mpz**
  %currCounter = load %mpz*, %mpz** %mpzPtrPtr
  %nextCounter = call %mpz* @hook_INT_add(%mpz* %currCounter, %mpz* @fresh_int_1)
  store %mpz* %nextCounter, %mpz** %mpzPtrPtr
  %sortData = call i8* @get_terminated_string(%string* %sort)
  %tag = call i32 @get_tag_for_fresh_sort(i8* %sortData)
  %args = alloca i8*
  %voidPtr = bitcast %mpz* %currCounter to i8*
  store i8* %voidPtr, i8** %args
  %retval = call i8* @evaluate_function_symbol(i32 %tag, i8** %args)
  ret i8* %retval
}

; get_tag

define i32 @get_tag(%block* %arg) {
  %intptr = ptrtoint %block* %arg to i64
  %isConstant = trunc i64 %intptr to i1
  br i1 %isConstant, label %constant, label %block
constant:
  %taglong = lshr i64 %intptr, 32
  br label %exit
block:
  %hdrptr = getelementptr inbounds %block, %block* %arg, i64 0, i32 0, i32 0
  %hdr = load i64, i64* %hdrptr
  %layout = lshr i64 %hdr, @LAYOUT_OFFSET@
  %isstring = icmp eq i64 %layout, 0
  %tagorstring = select i1 %isstring, i64 -1, i64 %hdr
  br label %exit
exit:
  %phi = phi i64 [ %tagorstring, %block ], [ %taglong, %constant ]
  %tag = trunc i64 %phi to i32
  ret i32 %tag
}

; helper function for float hooks
define %floating* @move_float(%floating* %val) {
  %loaded = load %floating, %floating* %val
  %malloccall = tail call i8* @kore_alloc_floating(i64 0)
  %ptr = bitcast i8* %malloccall to %floating*
  store %floating %loaded, %floating* %ptr
  ret %floating* %ptr

}

declare noalias i8* @kore_alloc_floating(i64)

; helper function for int hooks
define %mpz* @move_int(%mpz* %val) {
  %loaded = load %mpz, %mpz* %val
  %malloccall = tail call i8* @kore_alloc_integer(i64 0)
  %ptr = bitcast i8* %malloccall to %mpz*
  store %mpz %loaded, %mpz* %ptr
  ret %mpz* %ptr
}

declare noalias i8* @kore_alloc_integer(i64)

; string equality

declare i32 @memcmp(i8*, i8*, i64);

define i1 @string_equal(i8* %str1, i8* %str2, i64 %len1, i64 %len2) {
  %len_eq = icmp eq i64 %len1, %len2
  %len_lt = icmp ult i64 %len1, %len2
  %min_len = select i1 %len_lt, i64 %len1, i64 %len2
  %result = call i32 @memcmp(i8* %str1, i8* %str2, i64 %min_len)
  %prefix_eq = icmp eq i32 %result, 0
  %retval = and i1 %len_eq, %prefix_eq
  ret i1 %retval
}

; take_steps

declare tailcc %block* @k_step(%block*)
declare tailcc %block** @step_all(%block*, i64*)
declare void @serialize_configuration_to_file_v2(i8*, %block*)
declare void @write_uint64_to_file(i8*, i64)

@proof_output = external global i1
@output_file = external global i8*
@depth = thread_local global i64 zeroinitializer
@steps = thread_local global i64 zeroinitializer
@current_interval = thread_local global i64 0
@GC_THRESHOLD = thread_local global i64 @GC_THRESHOLD@

@gc_roots = global [256 x i8 *] zeroinitializer

define void @set_gc_threshold(i64 %threshold) {
  store i64 %threshold, i64* @GC_THRESHOLD
  ret void
}

define i64 @get_gc_threshold() {
  %threshold = load i64, i64* @GC_THRESHOLD
  ret i64 %threshold
}

define i1 @finished_rewriting() {
entry:
  %depth = load i64, i64* @depth
  %hasDepth = icmp sge i64 %depth, 0
  %steps = load i64, i64* @steps
  %stepsPlusOne = add i64 %steps, 1
  store i64 %stepsPlusOne, i64* @steps
  br i1 %hasDepth, label %if, label %else
if:
  %depthMinusOne = sub i64 %depth, 1
  store i64 %depthMinusOne, i64* @depth
  %finished = icmp eq i64 %depth, 0
  ret i1 %finished
else:
  ret i1 false
}

define %block* @take_steps(i64 %depth, %block* %subject) {
  %proof_output = load i1, i1* @proof_output
  br i1 %proof_output, label %if, label %merge
if:
  %output_file = load i8*, i8** @output_file
  call void @write_uint64_to_file(i8* %output_file, i64 18446744073709551615)
  call void @serialize_configuration_to_file_v2(i8* %output_file, %block* %subject)
  br label %merge
merge:
  store i64 %depth, i64* @depth
  %result = call tailcc %block* @k_step(%block* %subject)
  ret %block* %result
}

define %block** @take_search_step(%block* %subject, i64* %count) {
  store i64 -1, i64* @depth
  %result = call tailcc %block** @step_all(%block* %subject, i64* %count)
  ret %block** %result
}

define i64 @get_steps() {
entry:
  %steps = load i64, i64* @steps
  ret i64 %steps
}

attributes #0 = { noreturn }  
)LLVM"
