; ModuleID = 'test16.ll'
source_filename = "test16.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [6 x i8] c"dummy\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @packet_get_int_safe() #0 {
entry:
  ret i32 10
}

; Function Attrs: noinline nounwind uwtable
define dso_local i8* @packet_get_string_safe() #0 {
entry:
  ret i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0)
}

; Function Attrs: noinline nounwind uwtable
define dso_local zeroext i1 @check_mul_overflow_int(i32 noundef %a, i32 noundef %b) #0 {
entry:
  %cmp = icmp sgt i32 %a, 0
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %cmp1 = icmp sgt i32 %b, 0
  br i1 %cmp1, label %land.lhs.true2, label %if.end

land.lhs.true2:                                   ; preds = %land.lhs.true
  %div = sdiv i32 2147483647, %b
  %cmp3 = icmp sgt i32 %a, %div
  br i1 %cmp3, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true2
  br label %return

if.end:                                           ; preds = %land.lhs.true2, %land.lhs.true, %entry
  %cmp4 = icmp slt i32 %a, 0
  br i1 %cmp4, label %land.lhs.true5, label %if.end11

land.lhs.true5:                                   ; preds = %if.end
  %cmp6 = icmp slt i32 %b, 0
  br i1 %cmp6, label %land.lhs.true7, label %if.end11

land.lhs.true7:                                   ; preds = %land.lhs.true5
  %div8 = sdiv i32 2147483647, %b
  %cmp9 = icmp slt i32 %a, %div8
  br i1 %cmp9, label %if.then10, label %if.end11

if.then10:                                        ; preds = %land.lhs.true7
  br label %return

if.end11:                                         ; preds = %land.lhs.true7, %land.lhs.true5, %if.end
  %cmp12 = icmp sgt i32 %a, 0
  br i1 %cmp12, label %land.lhs.true13, label %if.end19

land.lhs.true13:                                  ; preds = %if.end11
  %cmp14 = icmp slt i32 %b, 0
  br i1 %cmp14, label %land.lhs.true15, label %if.end19

land.lhs.true15:                                  ; preds = %land.lhs.true13
  %div16 = sdiv i32 -2147483648, %a
  %cmp17 = icmp slt i32 %b, %div16
  br i1 %cmp17, label %if.then18, label %if.end19

if.then18:                                        ; preds = %land.lhs.true15
  br label %return

if.end19:                                         ; preds = %land.lhs.true15, %land.lhs.true13, %if.end11
  %cmp20 = icmp slt i32 %a, 0
  br i1 %cmp20, label %land.lhs.true21, label %if.end27

land.lhs.true21:                                  ; preds = %if.end19
  %cmp22 = icmp sgt i32 %b, 0
  br i1 %cmp22, label %land.lhs.true23, label %if.end27

land.lhs.true23:                                  ; preds = %land.lhs.true21
  %div24 = sdiv i32 -2147483648, %b
  %cmp25 = icmp slt i32 %a, %div24
  br i1 %cmp25, label %if.then26, label %if.end27

if.then26:                                        ; preds = %land.lhs.true23
  br label %return

if.end27:                                         ; preds = %land.lhs.true23, %land.lhs.true21, %if.end19
  br label %return

return:                                           ; preds = %if.end27, %if.then26, %if.then18, %if.then10, %if.then
  %retval.0 = phi i1 [ true, %if.then ], [ true, %if.then10 ], [ true, %if.then18 ], [ true, %if.then26 ], [ false, %if.end27 ]
  ret i1 %retval.0
}

; Function Attrs: noinline nounwind uwtable
define dso_local void @test_nresp_overflow_good() #0 {
entry:
  %call = call i32 @packet_get_int_safe()
  %cmp = icmp sgt i32 %call, 0
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %call1 = call zeroext i1 @check_mul_overflow_int(i32 noundef %call, i32 noundef 8)
  br i1 %call1, label %if.end, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %mul = mul nsw i32 %call, 8
  %conv = sext i32 %mul to i64
  %call2 = call noalias i8* @malloc(i64 noundef %conv) #2
  %0 = bitcast i8* %call2 to i8**
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %entry
  %response.0 = phi i8** [ null, %land.lhs.true ], [ %0, %if.then ], [ null, %entry ]
  %tobool = icmp ne i8** %response.0, null
  br i1 %tobool, label %if.then3, label %if.end7

if.then3:                                         ; preds = %if.end
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then3
  %i.0 = phi i32 [ 0, %if.then3 ], [ %inc, %for.inc ]
  %cmp4 = icmp slt i32 %i.0, %call
  br i1 %cmp4, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call6 = call i8* @packet_get_string_safe()
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i8*, i8** %response.0, i64 %idxprom
  store i8* %call6, i8** %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !6

for.end:                                          ; preds = %for.cond
  %1 = bitcast i8** %response.0 to i8*
  call void @free(i8* noundef %1) #2
  br label %if.end7

if.end7:                                          ; preds = %for.end, %if.end
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64 noundef) #1

; Function Attrs: nounwind
declare void @free(i8* noundef) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  call void @test_nresp_overflow_good()
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
