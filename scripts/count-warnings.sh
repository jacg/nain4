COMPILATION_LOG=$1
ACCEPTABLE_NUMBER_OF_WARNINGS=$2
FAIL=$3

WARNING_COUNT=$(grep -io warning $COMPILATION_LOG | wc -l)
if (( $WARNING_COUNT > $ACCEPTABLE_NUMBER_OF_WARNINGS ))
then
    echo too many warnings: $WARNING_COUNT
    echo
    echo
    grep -iC2 warning compilation-log
    false || [ "$FAIL" != "fail" ]
else
    echo Warnings detected: $WARNING_COUNT
    echo We tolerate up to $ACCEPTABLE_NUMBER_OF_WARNINGS warnings
    true
fi
