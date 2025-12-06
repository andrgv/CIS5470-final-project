; ModuleID = 'test15.c'
source_filename = "test15.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"dummy\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @packet_get_int() #0 {
entry:
  ret i32 268435456
}

; Function Attrs: noinline nounwind uwtable
define dso_local i8* @packet_get_string() #0 {
entry:
  ret i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0)
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @test_nresp_overflow_bad() #0 {
entry:
  %i = alloca i32, align 4
  %nresp = alloca i32, align 4
  %response = alloca i8**, align 8
  %call = call i32 @packet_get_int()
  store i32 %call, i32* %nresp, align 4
  %0 = load i32, i32* %nresp, align 4
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load i32, i32* %nresp, align 4
  %mul = mul nsw i32 %1, 8
  %conv = sext i32 %mul to i64
  %call1 = call noalias i8* @malloc(i64 noundef %conv) #2
  %2 = bitcast i8* %call1 to i8**
  store i8** %2, i8*** %response, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then
  %3 = load i32, i32* %i, align 4
  %4 = load i32, i32* %nresp, align 4
  %cmp2 = icmp slt i32 %3, %4
  br i1 %cmp2, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call4 = call i8* @packet_get_string()
  %5 = load i8**, i8*** %response, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds i8*, i8** %5, i64 %idxprom
  store i8* %call4, i8** %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, i32* %i, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond, !llvm.loop !6

for.end:                                          ; preds = %for.cond
  %8 = load i8**, i8*** %response, align 8
  %9 = bitcast i8** %8 to i8*
  call void @free(i8* noundef %9) #2
  br label %if.end

if.end:                                           ; preds = %for.end, %entry
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64 noundef) #1

; Function Attrs: nounwind
declare void @free(i8* noundef) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @test_nresp_overflow_bad()
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 14.0.6"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
