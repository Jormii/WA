import os
import sys

import matplotlib.pyplot as plt
import numpy as np

EXTENSIONS = [".png"]


def main() -> int:
    argv = sys.argv
    argc = len(argv)

    exit = False
    if argc != 2:
        exit = True
    else:
        img_file = argv[1]
        _, ext = os.path.splitext(img_file)

        exit = ext not in EXTENSIONS

    if exit:
        print(f"Usage: {os.path.basename(__file__)} <{' | '.join(EXTENSIONS)}>")
        return 1

    assert os.path.isfile(img_file), f"{img_file} does not exist"

    img = plt.imread(img_file)
    match img.dtype.name:
        case "float32":
            img = np.round(255.0 * img).astype("uint8")
        case _:
            raise NotImplementedError(img.dtype.name)

    assert img.ndim == 3
    assert img.shape[2] == 4
    assert img.dtype.name == "uint8"

    rows, cols, _ = img.shape
    assert rows > 0
    assert cols > 0

    R = img[:, :, 0]
    G = img[:, :, 1]
    B = img[:, :, 2]
    A = img[:, :, 3]
    assert np.all((R >= 0) & (R <= 255))
    assert np.all((G >= 0) & (G <= 255))
    assert np.all((B >= 0) & (B <= 255))
    assert np.all((A >= 0) & (A <= 255))

    dir, filename = os.path.split(img_file)
    filename_no_ext, _ = os.path.splitext(filename)
    hpp_file = os.path.join(dir, f"{filename}.hpp")
    with open(hpp_file, mode="w") as fd:
        fd.write("#pragma once\n")
        fd.write('\n#include "types.hpp"\n')

        fd.write(f"\nArr2D<{rows}, {cols}, RGBA> {filename_no_ext}_texture = {{{{\n")
        for i in range(rows):
            fd.write(f"\t// Row {i}\n")

            for j in range(cols):
                r = R[i, j]
                g = G[i, j]
                b = B[i, j]
                a = A[i, j]

                fd.write(f"\t{{{r}, {g}, {b}, {a}}}, // {i}, {j}\n")

        fd.write("}};\n")

    return 0


if __name__ == "__main__":
    exit(main())
