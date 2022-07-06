target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%blockheader = type { i64 }
%mpz = type { i32, i32, i64* }
%block = type { %blockheader, [0 x i64*] }
%map = type { { i8*, i64 } }
%list = type { { i64, i32, i8*, i8* } }
%set = type { { i8*, i64 } }
%floating = type { i64, { i64, i32, i64, i64* } }
%string = type { %blockheader, [0 x i8] }

; this file contains a set of functions templated by type that fall into 4 basic categories:
; * address space cast from addrspace(1) to addrspace(0)
; * address space cast from addrspace(0) to addrspace(1)
; * ptrtoint cast
; * inttoptr cast
;
; the reason these instructions live in separate functions is because the RewriteStatepointsForGC
; pass doesn't understand these instructions, and would optimize incorrectly if it knew these
; instructions were being called. The intent is that after performing all the regular optimization
; passes, we will use llvm-link to link this file together with the file generated by code generation
; and then run the always inline llvm pass to inline these instructions, so that at the end of the day,
; they get implemented as a no-op in the resulting binary.

define %block addrspace(1)* addrspace(1)* @addrspace_0_to_1_p1s_blocks(%block addrspace(1)** %in) #0 {
  %out = addrspacecast %block addrspace(1)** %in to %block addrspace(1)* addrspace(1)*
  ret %block addrspace(1)* addrspace(1)* %out
}

define %block addrspace(8)* @addrspace_0_to_8_s_blocks(%block* %in) #0 {
  %out = addrspacecast %block* %in to %block addrspace(8)*
  ret %block addrspace(8)* %out
}

define %mpz addrspace(4)* @addrspace_0_to_4_s_mpzs(%mpz* %in) #0 {
  %out = addrspacecast %mpz* %in to %mpz addrspace(4)*
  ret %mpz addrspace(4)* %out
}

define %floating addrspace(5)* @addrspace_0_to_5_s_floatings(%floating* %in) #0 {
  %out = addrspacecast %floating* %in to %floating addrspace(5)*
  ret %floating addrspace(5)* %out
}

define %map addrspace(8)* @addrspace_0_to_8_s_maps(%map* %in) #0 {
  %out = addrspacecast %map* %in to %map addrspace(8)*
  ret %map addrspace(8)* %out
}

define %list addrspace(8)* @addrspace_0_to_2_s_lists(%list* %in) #0 {
  %out = addrspacecast %list* %in to %list addrspace(8)*
  ret %list addrspace(8)* %out
}

define %set addrspace(8)* @addrspace_0_to_3_s_sets(%set* %in) #0 {
  %out = addrspacecast %set* %in to %set addrspace(8)*
  ret %set addrspace(8)* %out
}

define %string addrspace(1)* @addrspace_0_to_1_s_strings(%string* %in) #0 {
  %out = addrspacecast %string* %in to %string addrspace(1)*
  ret %string addrspace(1)* %out
}

define i8 addrspace(4)* @addrspace_0_to_4_i8(i8* %in) #0 {
  %out = addrspacecast i8* %in to i8 addrspace(4)*
  ret i8 addrspace(4)* %out
}

define i8 addrspace(5)* @addrspace_0_to_5_i8(i8* %in) #0 {
  %out = addrspacecast i8* %in to i8 addrspace(5)*
  ret i8 addrspace(5)* %out
}

define i8 addrspace(6)* @addrspace_0_to_6_i8(i8* %in) #0 {
  %out = addrspacecast i8* %in to i8 addrspace(6)*
  ret i8 addrspace(6)* %out
}

define i8 addrspace(7)* @addrspace_0_to_7_i8(i8* %in) #0 {
  %out = addrspacecast i8* %in to i8 addrspace(7)*
  ret i8 addrspace(7)* %out
}

define i8 addrspace(8)* @addrspace_0_to_8_i8(i8* %in) #0 {
  %out = addrspacecast i8* %in to i8 addrspace(8)*
  ret i8 addrspace(8)* %out
}

define i8 addrspace(10)* @addrspace_0_to_10_i8(i8* %in) #0 {
  %out = addrspacecast i8* %in to i8 addrspace(10)*
  ret i8 addrspace(10)* %out
}

define i1 addrspace(4)* @addrspace_0_to_4_i1(i1* %in) #0 {
  %out = addrspacecast i1* %in to i1 addrspace(4)*
  ret i1 addrspace(4)* %out
}

define i1 addrspace(5)* @addrspace_0_to_5_i1(i1* %in) #0 {
  %out = addrspacecast i1* %in to i1 addrspace(5)*
  ret i1 addrspace(5)* %out
}

define i32 addrspace(4)* @addrspace_0_to_4_i32(i32* %in) #0 {
  %out = addrspacecast i32* %in to i32 addrspace(4)*
  ret i32 addrspace(4)* %out
}

define i32 addrspace(5)* @addrspace_0_to_5_i32(i32* %in) #0 {
  %out = addrspacecast i32* %in to i32 addrspace(5)*
  ret i32 addrspace(5)* %out
}

define i64 addrspace(4)* @addrspace_0_to_4_i64(i64* %in) #0 {
  %out = addrspacecast i64* %in to i64 addrspace(4)*
  ret i64 addrspace(4)* %out
}

define i64 addrspace(5)* @addrspace_0_to_5_i64(i64* %in) #0 {
  %out = addrspacecast i64* %in to i64 addrspace(5)*
  ret i64 addrspace(5)* %out
}

define i8* @addrspace_4_to_0_i8(i8 addrspace(4)* %in) #0 {
  %out = addrspacecast i8 addrspace(4)* %in to i8*
  ret i8* %out
}

define i8* @addrspace_5_to_0_i8(i8 addrspace(5)* %in) #0 {
  %out = addrspacecast i8 addrspace(5)* %in to i8*
  ret i8* %out
}

define i8* @addrspace_6_to_0_i8(i8 addrspace(6)* %in) #0 {
  %out = addrspacecast i8 addrspace(6)* %in to i8*
  ret i8* %out
}

define i8* @addrspace_8_to_0_i8(i8 addrspace(8)* %in) #0 {
  %out = addrspacecast i8 addrspace(8)* %in to i8*
  ret i8* %out
}

define %block addrspace(8)* @inttoptr_i64.p8s_blocks(i64 %in) #0 {
  %out = inttoptr i64 %in to %block addrspace(8)*
  ret %block addrspace(8)* %out
}

define i64 @ptrtoint_i64.p8s_blocks(%block addrspace(8)* %in) #0 {
  %out = ptrtoint %block addrspace(8)* %in to i64
  ret i64 %out
}

attributes #0 = { alwaysinline "gc-leaf-function" }
