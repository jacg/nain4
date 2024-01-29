COMPILATION_LOG=$1
TOLERATED_WARNINGS_FILE=$2
FAIL=$3

readarray -t TOLERATED_WARNINGS < $(rg --no-line-number -v '^\s*(#|$)' $TOLERATED_WARNINGS_FILE)

echo 'Tolerated warnings (one per line)':
for w in "${TOLERATED_WARNINGS[@]}";do echo $w; done

WARNINGS=$(grep -i warning $COMPILATION_LOG)

for TOLERATED_WARNING in "${TOLERATED_WARNINGS[@]}"; do
    WARNINGS=$(echo $WARNINGS | grep -v "$TOLERATED_WARNING")
done

WARNING_COUNT=$(echo $WARNINGS | grep -ve '^[[:space:]]*$' | wc -l)

if (( $WARNING_COUNT > 0 ))
then
    echo too many unexpected warnings: $WARNING_COUNT
    echo
    echo
    grep -iC2 warning compilation-log
    false || [ "$FAIL" != "fail" ]
else
    echo No untolerated warnings detected
    true
fi
