; ModuleID = 'test17.c'
source_filename = "test17.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local signext i16 @getFromInput(i8* noundef %buf) #0 {
entry:
  %buf.addr = alloca i8*, align 8
  store i8* %buf, i8** %buf.addr, align 8
  %0 = load i8*, i8** %buf.addr, align 8
  ret i16 1000
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @test_loop_overflow_bad() #0 {
entry:
  %bytesRec = alloca i16, align 2
  %buf = alloca [50000 x i8], align 16
  store i16 0, i16* %bytesRec, align 2
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i16, i16* %bytesRec, align 2
  %conv = sext i16 %0 to i32
  %cmp = icmp slt i32 %conv, 40000
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %arraydecay = getelementptr inbounds [50000 x i8], [50000 x i8]* %buf, i64 0, i64 0
  %1 = load i16, i16* %bytesRec, align 2
  %conv2 = sext i16 %1 to i32
  %idx.ext = sext i32 %conv2 to i64
  %add.ptr = getelementptr inbounds i8, i8* %arraydecay, i64 %idx.ext
  %call = call signext i16 @getFromInput(i8* noundef %add.ptr)
  %conv3 = sext i16 %call to i32
  %2 = load i16, i16* %bytesRec, align 2
  %conv4 = sext i16 %2 to i32
  %add = add nsw i32 %conv4, %conv3
  %conv5 = trunc i32 %add to i16
  store i16 %conv5, i16* %bytesRec, align 2
  br label %while.cond, !llvm.loop !6

while.end:                                        ; preds = %while.cond
  ret void
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @test_loop_overflow_bad()
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

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
