; ModuleID = 'quick_pow.c'
source_filename = "quick_pow.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @quick_pow(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 %0, ptr %5, align 4
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  store i32 1, ptr %8, align 4
  %9 = load i32, ptr %5, align 4
  %10 = load i32, ptr %7, align 4
  %11 = urem i32 %9, %10
  store i32 %11, ptr %5, align 4
  %12 = load i32, ptr %6, align 4
  %13 = icmp ne i32 %12, 2
  br i1 %13, label %14, label %15

14:                                               ; preds = %3
  store i32 0, ptr %4, align 4
  br label %39

15:                                               ; preds = %3
  br label %16

16:                                               ; preds = %29, %15
  %17 = load i32, ptr %6, align 4
  %18 = icmp ugt i32 %17, 0
  br i1 %18, label %19, label %37

19:                                               ; preds = %16
  %20 = load i32, ptr %6, align 4
  %21 = urem i32 %20, 2
  %22 = icmp eq i32 %21, 1
  br i1 %22, label %23, label %29

23:                                               ; preds = %19
  %24 = load i32, ptr %8, align 4
  %25 = load i32, ptr %5, align 4
  %26 = mul i32 %24, %25
  %27 = load i32, ptr %7, align 4
  %28 = urem i32 %26, %27
  store i32 %28, ptr %8, align 4
  br label %29

29:                                               ; preds = %23, %19
  %30 = load i32, ptr %5, align 4
  %31 = load i32, ptr %5, align 4
  %32 = mul i32 %30, %31
  %33 = load i32, ptr %7, align 4
  %34 = urem i32 %32, %33
  store i32 %34, ptr %5, align 4
  %35 = load i32, ptr %6, align 4
  %36 = udiv i32 %35, 2
  store i32 %36, ptr %6, align 4
  br label %16, !llvm.loop !6

37:                                               ; preds = %16
  %38 = load i32, ptr %8, align 4
  store i32 %38, ptr %4, align 4
  br label %39

39:                                               ; preds = %37, %14
  %40 = load i32, ptr %4, align 4
  ret i32 %40
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i64, align 8
  %3 = alloca i64, align 8
  %4 = alloca i64, align 8
  %5 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  store i64 2, ptr %2, align 8
  store i64 10, ptr %3, align 8
  store i64 987654321, ptr %4, align 8
  store i32 1000000, ptr %5, align 4
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6 (git@github.com:llvm/llvm-project.git 6009708b4367171ccdbf4b5905cb6a803753fe18)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
