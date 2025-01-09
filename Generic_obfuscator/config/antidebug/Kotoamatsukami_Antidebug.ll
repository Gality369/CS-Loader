; ModuleID = 'KObfucator_Antidebug.c'
source_filename = "KObfucator_Antidebug.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [20 x i8] c"Debugger detected!\0A\00", align 1
@.str.1 = private unnamed_addr constant [37 x i8] c"No debugger detected, continuing...\0A\00", align 1
@.str.2 = private unnamed_addr constant [61 x i8] c"Debugger detected based on session and parent PID mismatch!\0A\00", align 1
@.str.3 = private unnamed_addr constant [2 x i8] c"_\00", align 1
@.str.4 = private unnamed_addr constant [42 x i8] c"No previous command found, continuing...\0A\00", align 1
@.str.5 = private unnamed_addr constant [4 x i8] c"gdb\00", align 1
@.str.6 = private unnamed_addr constant [7 x i8] c"strace\00", align 1
@.str.7 = private unnamed_addr constant [7 x i8] c"ltrace\00", align 1
@.str.8 = private unnamed_addr constant [44 x i8] c"Debugger detected by environment variable!\0A\00", align 1
@.str.9 = private unnamed_addr constant [51 x i8] c"Debugger detected due to delayed signal handling!\0A\00", align 1
@.str.10 = private unnamed_addr constant [30 x i8] c"Debugger detected by ptrace!\0A\00", align 1
@.str.11 = private unnamed_addr constant [17 x i8] c"/proc/%d/cmdline\00", align 1
@.str.12 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str.13 = private unnamed_addr constant [38 x i8] c"Debugger detected by parent process!\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @KObfucator_Antidebug1() #0 {
  %1 = call i64 (i32, ...) @ptrace(i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0) #5
  %2 = icmp slt i64 %1, 0
  br i1 %2, label %3, label %5

3:                                                ; preds = %0
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  call void @_exit(i32 noundef 1) #6
  unreachable

5:                                                ; preds = %0
  %6 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  ret void
}

; Function Attrs: nounwind
declare i64 @ptrace(i32 noundef, ...) #1

declare i32 @printf(ptr noundef, ...) #2

; Function Attrs: noreturn
declare void @_exit(i32 noundef) #3

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @KObfucator_Antidebug2() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = call i32 @getppid() #5
  store i32 %3, ptr %1, align 4
  %4 = call i32 @getpid() #5
  %5 = call i32 @getsid(i32 noundef %4) #5
  store i32 %5, ptr %2, align 4
  %6 = load i32, ptr %2, align 4
  %7 = load i32, ptr %1, align 4
  %8 = icmp ne i32 %6, %7
  br i1 %8, label %9, label %11

9:                                                ; preds = %0
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  call void @_exit(i32 noundef 1) #6
  unreachable

11:                                               ; preds = %0
  %12 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  br label %13

13:                                               ; preds = %11
  ret void
}

; Function Attrs: nounwind
declare i32 @getppid() #1

; Function Attrs: nounwind
declare i32 @getsid(i32 noundef) #1

; Function Attrs: nounwind
declare i32 @getpid() #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @KObfucator_Antidebug3() #0 {
  %1 = call ptr @signal(i32 noundef 5, ptr noundef inttoptr (i64 1 to ptr)) #5
  call void asm sideeffect "nop\0A\09int3\0A\09", "~{dirflag},~{fpsr},~{flags}"() #5, !srcloc !6
  %2 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  ret void
}

; Function Attrs: nounwind
declare ptr @signal(i32 noundef, ptr noundef) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @KObfucator_Antidebug4() #0 {
  %1 = alloca ptr, align 8
  %2 = call ptr @getenv(ptr noundef @.str.3) #5
  store ptr %2, ptr %1, align 8
  %3 = load ptr, ptr %1, align 8
  %4 = icmp eq ptr %3, null
  br i1 %4, label %5, label %7

5:                                                ; preds = %0
  %6 = call i32 (ptr, ...) @printf(ptr noundef @.str.4)
  br label %23

7:                                                ; preds = %0
  %8 = load ptr, ptr %1, align 8
  %9 = call i32 @strcmp(ptr noundef %8, ptr noundef @.str.5) #7
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %19, label %11

11:                                               ; preds = %7
  %12 = load ptr, ptr %1, align 8
  %13 = call i32 @strcmp(ptr noundef %12, ptr noundef @.str.6) #7
  %14 = icmp eq i32 %13, 0
  br i1 %14, label %19, label %15

15:                                               ; preds = %11
  %16 = load ptr, ptr %1, align 8
  %17 = call i32 @strcmp(ptr noundef %16, ptr noundef @.str.7) #7
  %18 = icmp eq i32 %17, 0
  br i1 %18, label %19, label %21

19:                                               ; preds = %15, %11, %7
  %20 = call i32 (ptr, ...) @printf(ptr noundef @.str.8)
  call void @_exit(i32 noundef 1) #6
  unreachable

21:                                               ; preds = %15
  %22 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  br label %23

23:                                               ; preds = %21, %5
  ret void
}

; Function Attrs: nounwind
declare ptr @getenv(ptr noundef) #1

; Function Attrs: nounwind willreturn memory(read)
declare i32 @strcmp(ptr noundef, ptr noundef) #4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @alarm_handler(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str.9)
  call void @_exit(i32 noundef 1) #6
  unreachable
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @KObfucator_Antidebug5() #0 {
  %1 = call ptr @signal(i32 noundef 14, ptr noundef @alarm_handler) #5
  %2 = call i32 @alarm(i32 noundef 1) #5
  br label %3

3:                                                ; preds = %0, %3
  br label %3
}

; Function Attrs: nounwind
declare i32 @alarm(i32 noundef) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @KObfucator_Antidebug6() #0 {
  %1 = call i64 (i32, ...) @ptrace(i32 noundef 0, i32 noundef 0, i32 noundef 0, i32 noundef 0) #5
  %2 = icmp slt i64 %1, 0
  br i1 %2, label %3, label %5

3:                                                ; preds = %0
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str.10)
  call void @_exit(i32 noundef 1) #6
  unreachable

5:                                                ; preds = %0
  %6 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @KObfucator_Antidebug7() #0 {
  %1 = alloca i32, align 4
  %2 = alloca [1024 x i8], align 16
  %3 = alloca [256 x i8], align 16
  %4 = alloca ptr, align 8
  %5 = call i32 @getppid() #5
  store i32 %5, ptr %1, align 4
  %6 = getelementptr inbounds [256 x i8], ptr %3, i64 0, i64 0
  %7 = load i32, ptr %1, align 4
  %8 = call i32 (ptr, i64, ptr, ...) @snprintf(ptr noundef %6, i64 noundef 256, ptr noundef @.str.11, i32 noundef %7) #5
  %9 = getelementptr inbounds [256 x i8], ptr %3, i64 0, i64 0
  %10 = call noalias ptr @fopen(ptr noundef %9, ptr noundef @.str.12)
  store ptr %10, ptr %4, align 8
  %11 = load ptr, ptr %4, align 8
  %12 = icmp ne ptr %11, null
  br i1 %12, label %13, label %19

13:                                               ; preds = %0
  %14 = getelementptr inbounds [1024 x i8], ptr %2, i64 0, i64 0
  %15 = load ptr, ptr %4, align 8
  %16 = call ptr @fgets(ptr noundef %14, i32 noundef 1024, ptr noundef %15)
  %17 = load ptr, ptr %4, align 8
  %18 = call i32 @fclose(ptr noundef %17)
  br label %19

19:                                               ; preds = %13, %0
  %20 = getelementptr inbounds [1024 x i8], ptr %2, i64 0, i64 0
  %21 = call ptr @strstr(ptr noundef %20, ptr noundef @.str.5) #7
  %22 = icmp ne ptr %21, null
  br i1 %22, label %31, label %23

23:                                               ; preds = %19
  %24 = getelementptr inbounds [1024 x i8], ptr %2, i64 0, i64 0
  %25 = call ptr @strstr(ptr noundef %24, ptr noundef @.str.6) #7
  %26 = icmp ne ptr %25, null
  br i1 %26, label %31, label %27

27:                                               ; preds = %23
  %28 = getelementptr inbounds [1024 x i8], ptr %2, i64 0, i64 0
  %29 = call ptr @strstr(ptr noundef %28, ptr noundef @.str.7) #7
  %30 = icmp ne ptr %29, null
  br i1 %30, label %31, label %33

31:                                               ; preds = %27, %23, %19
  %32 = call i32 (ptr, ...) @printf(ptr noundef @.str.13)
  call void @_exit(i32 noundef 1) #6
  unreachable

33:                                               ; preds = %27
  %34 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  ret void
}

; Function Attrs: nounwind
declare i32 @snprintf(ptr noundef, i64 noundef, ptr noundef, ...) #1

declare noalias ptr @fopen(ptr noundef, ptr noundef) #2

declare ptr @fgets(ptr noundef, i32 noundef, ptr noundef) #2

declare i32 @fclose(ptr noundef) #2

; Function Attrs: nounwind willreturn memory(read)
declare ptr @strstr(ptr noundef, ptr noundef) #4

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { noreturn "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind willreturn memory(read) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind }
attributes #6 = { noreturn }
attributes #7 = { nounwind willreturn memory(read) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 17.0.6 (git@github.com:llvm/llvm-project.git 6009708b4367171ccdbf4b5905cb6a803753fe18)"}
!6 = !{i64 704, i64 710, i64 733}
