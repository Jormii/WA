make -B
if [ $? -eq 0 ]; then
    DIR="bin"

    mkdir -p $DIR
    mv EBOOT.PBP PARAM.SFO WA.elf WA.prx $DIR
fi