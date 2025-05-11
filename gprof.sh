set -e

NOW=$(date +%s)
for BUILD in "TEST" "DEBUG" "RELEASE"; do
    DIR=bin/$BUILD

    if [[ -f $DIR/gmon.out ]]; then
        psp-gprof -b $DIR/WA_$BUILD.elf $DIR/gmon.out > gmon_$BUILD.$NOW.txt
        rm $DIR/gmon.out
    fi
done
