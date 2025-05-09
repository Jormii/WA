set -e

PRX=1
PPSSPP=0
DEBUGGER=0

if [[ $PPSSPP -eq 1 ]]; then
    DEBUGGER=0
fi

python C-CPP-CodeBase/compile_tests.py C-CPP-CodeBase/tests tests 
# for BUILD in "TEST"; do
for BUILD in "TEST" "DEBUG" "RELEASE"; do
    make -B PRX=$PRX BUILD=$BUILD PPSSPP=$PPSSPP DEBUGGER=$DEBUGGER

    DIR=bin/$BUILD
    mkdir -p $DIR
    mv EBOOT.PBP PARAM.SFO WA_$BUILD.elf $DIR
    if [[ $PRX -eq 1 ]]; then
        mv WA_$BUILD.prx $DIR
    elif [[ -f $DIR/WA_$BUILD.prx ]]; then
        rm $DIR/WA_$BUILD.prx
    fi
done
