set -e

python C-CPP-CodeBase/compile_tests.py C-CPP-CodeBase/tests tests 
# for BUILD in "TEST"; do
for BUILD in "TEST" "DEBUG" "RELEASE"; do
    make -B BUILD=$BUILD

    DIR=bin/$BUILD
    mkdir -p $DIR
    mv EBOOT.PBP PARAM.SFO WA_$BUILD.elf WA_$BUILD.prx $DIR
done
