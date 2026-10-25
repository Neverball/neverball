#!/usr/bin/env python3
"""
Convert Neverball Wavefront OBJ models from legacy OpenGL coordinates
(Y-up, in meters) to TrenchBroom map coordinates (Quake Z-up, 64 units = 1m).

Transformation:
  X_new = -64 * Z_old
  Y_new = +64 * Y_old
  Z_new = +64 * X_old

Normals:
  NX_new = -NZ_old
  NY_new = +NY_old
  NZ_new = +NX_old
"""

import sys
import os

def convert_obj(file_path):
    with open(file_path, "r", encoding="utf-8", errors="ignore") as f:
        lines = f.readlines()

    out_lines = []
    for line in lines:
        if line.startswith("v "):
            parts = line.strip().split()
            if len(parts) >= 4:
                x = float(parts[1])
                y = float(parts[2])
                z = float(parts[3])
                nx = -64.0 * z
                ny = 64.0 * y
                nz = 64.0 * x
                # Format with up to 9 significant digits for exact float round-trip
                out_lines.append(f"v {nx:.9g} {ny:.9g} {nz:.9g}\n")
            else:
                out_lines.append(line)
        elif line.startswith("vn "):
            parts = line.strip().split()
            if len(parts) >= 4:
                nx_old = float(parts[1])
                ny_old = float(parts[2])
                nz_old = float(parts[3])
                nx = -nz_old
                ny = ny_old
                nz = nx_old
                out_lines.append(f"vn {nx:.9g} {ny:.9g} {nz:.9g}\n")
            else:
                out_lines.append(line)
        else:
            out_lines.append(line)

    with open(file_path, "w", encoding="utf-8") as f:
        f.writelines(out_lines)

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <file.obj> [...]", file=sys.stderr)
        sys.exit(1)

    for path in sys.argv[1:]:
        if os.path.isfile(path):
            convert_obj(path)
            print(f"Converted {path}")
        else:
            print(f"Skipped {path} (not a file)", file=sys.stderr)

if __name__ == "__main__":
    main()
