#!/usr/bin/env python3
import os
import subprocess
import sys


def main(root, out_file, map_out_file):
    out_dir = root

    for f in os.scandir(root):
        if f.name.find('cpp') > -1:
            f_split = f.name.split('.')

            name = '.'.join(f_split[0:len(f_split)-1])

            input_file = os.path.join(os.path.abspath(root), f.name)

            intermediate_file = os.path.join(out_dir, name + '-intermediate.wasm')
            wasm_output_file = out_file or os.path.join(out_dir, name + '.wasm')

            compile_to_intermediate(input_file, intermediate_file)
            link_wasm(intermediate_file, wasm_output_file)
            try:
                rename_map_file(name, out_dir)
            except FileNotFoundError:
                pass

            os.remove(intermediate_file)


def compile_to_intermediate(input_file, intermediate_file):
    res = subprocess.run(
        ['eosio-cpp', '-O0', '-c', input_file, '-o', intermediate_file],
        capture_output=True
    )
    res.check_returncode()


def link_wasm(intermediate_file, wasm_output_file):
    res = subprocess.run(
        ['eosio-ld', intermediate_file, '-o', wasm_output_file],
        capture_output=True
    )
    res.check_returncode()

def rename_map_file(name, out_dir):
    os.rename(os.path.join(out_dir, name + '.map'), os.path.join(out_dir, 'generated.map'))

if __name__ == "__main__":
    root_path = sys.argv[1]
    out_file_path = sys.argv[2] if sys.argv[2] else ""
    map_file_path = sys.argv[3] if sys.argv[3] else ""
    main(root_path, out_file_path, map_file_path)
