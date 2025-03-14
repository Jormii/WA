set -e

python "C-CPP-CodeBase/compile_tests.py"
for BUILD in "TEST"; do
# for BUILD in "TEST" "DEBUG"; do
    make -B BUILD=$BUILD

    DIR="bin/$BUILD"
    mkdir -p $DIR
    mv EBOOT.PBP PARAM.SFO WA_$BUILD.elf WA_$BUILD.prx $DIR
done
