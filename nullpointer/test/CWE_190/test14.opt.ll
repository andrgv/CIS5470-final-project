; ModuleID = 'test14.ll'
source_filename = "test14.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.img_t = type { [10240 x i8] }

; Function Attrs: noinline nounwind uwtable
define dso_local void @test_alloc_overflow_good() #0 {
entry:
  %cmp = icmp sgt i32 100, 0
  br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %entry
  %cmp1 = icmp sle i32 100, 209715
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %conv = sext i32 100 to i64
  %mul = mul i64 10240, %conv
  %call = call noalias i8* @malloc(i64 noundef %mul) #2
  %0 = bitcast i8* %call to %struct.img_t*
  br label %if.end

if.else:                                          ; preds = %land.lhs.true, %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %table_ptr.0 = phi %struct.img_t* [ %0, %if.then ], [ null, %if.else ]
  %tobool = icmp ne %struct.img_t* %table_ptr.0, null
  br i1 %tobool, label %if.then2, label %if.end4

if.then2:                                         ; preds = %if.end
  %arrayidx = getelementptr inbounds %struct.img_t, %struct.img_t* %table_ptr.0, i64 0
  %data = getelementptr inbounds %struct.img_t, %struct.img_t* %arrayidx, i32 0, i32 0
  %arrayidx3 = getelementptr inbounds [10240 x i8], [10240 x i8]* %data, i64 0, i64 0
  store i8 42, i8* %arrayidx3, align 1
  %1 = bitcast %struct.img_t* %table_ptr.0 to i8*
  call void @free(i8* noundef %1) #2
  br label %if.end4

if.end4:                                          ; preds = %if.then2, %if.end
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64 noundef) #1

; Function Attrs: nounwind
declare void @free(i8* noundef) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  call void @test_alloc_overflow_good()
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
