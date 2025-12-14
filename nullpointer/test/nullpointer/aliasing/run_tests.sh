#!/bin/bash

PASS_PLUGIN="../../../build/NullPtrPass.so"

TP=0; FP=0; TN=0; FN=0

declare -A ground_truth
ground_truth["test01"]="right"
ground_truth["test02"]="right"
ground_truth["test03"]="wrong"
ground_truth["test04"]="wrong"
ground_truth["test05"]="wrong"
ground_truth["test06"]="right"
ground_truth["test07"]="wrong"
ground_truth["test08"]="wrong"
ground_truth["test09"]="right"
ground_truth["test10"]="wrong"

echo "=============================================================="
echo "| Program   | Ground Truth | Detector     | Result          |"
echo "|============================================================|"

for i in {01..10}; do
    test_name="test$i"
    c_file="$test_name.c"
    ll_file="$test_name.ll"

    clang -emit-llvm -S -fno-discard-value-names -Xclang -disable-O0-optnone \
          -c -o "$ll_file" "$c_file"

    output=$(opt -load-pass-plugin="$PASS_PLUGIN" -passes="NullPtr" \
                 "$ll_file" -disable-output 2>&1)

    # Classification logic
    if echo "$output" | grep -q "Potential Instructions by NullPtr"; then
        
        trailing=$(echo "$output" | sed -n '/Potential Instructions by NullPtr/,$p' | tail -n +2)

        if [ -n "$trailing" ] && echo "$trailing" | grep -q '[^[:space:]]'; then
            detector_result="reject"
            detected_error=true
        else
            detector_result="accept"
            detected_error=false
        fi

    else
        detector_result="accept"
        detected_error=false
    fi

    # Metric updates
    expected=${ground_truth[$test_name]}

    if [ "$expected" = "wrong" ]; then
        if [ "$detected_error" = true ]; then
            ((TP++))
            analysis_result="Correct"
        else
            ((FN++))
            analysis_result="Incorrect"
        fi
    else
        if [ "$detected_error" = true ]; then
            ((FP++))
            analysis_result="Incorrect"
        else
            ((TN++))
            analysis_result="Correct"
        fi
    fi

    printf "| %-9s | %-12s | %-12s | %-14s |\n" \
        "$c_file" "$expected" "$detector_result" "$analysis_result"
done

echo "|============================================|"

# Metrics
precision=$(awk "BEGIN { if ($TP+$FP > 0) printf \"%.2f\", $TP/($TP+$FP); else print \"0.00\" }")
recall=$(awk "BEGIN { if ($TP+$FN > 0) printf \"%.2f\", $TP/($TP+$FN); else print \"0.00\" }")
f1=$(awk "BEGIN { p=$precision; r=$recall; if (p+r > 0) printf \"%.2f\", 2*p*r/(p+r); else print \"0.00\" }")

echo "| Precision | $precision |"
echo "| Recall    | $recall    |"
echo "| F-measure | $f1        |"
echo "=============================================="

echo "False positives: $FP"
echo "True positives: $TP"
echo "False negatives: $FN"
echo "True negatives: $TN"
