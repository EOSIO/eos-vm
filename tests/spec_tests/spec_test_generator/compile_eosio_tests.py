#!/usr/bin/env python3
import os
import subprocess
import sys


def main():
    root = sys.argv[1]
    out_file = ""
    if len(sys.argv) > 2:
        out_file = sys.argv[2]

    #if len(sys.argv) >= 3 and sys.argv[2]:
        #out_dir = sys.argv[2]
    #else:
    out_dir = root
    try:
        os.mkdir(out_dir)
    except:
        pass

    for f in os.scandir(root):
        if (f.name.find('cpp') > -1):
            f_split = f.name.split('.')
            name = '.'.join(f_split[0:len(f_split)-1])

            input_file = os.path.join(os.path.abspath(root), f.name)

            intermediate_file = os.path.join(out_dir, name + '-intermediate.wasm')
            wasm_output_file = out_file or os.path.join(out_dir, name + '.wasm')

            compile_to_intermediate(input_file, intermediate_file)
            link_wasm(intermediate_file, wasm_output_file)

            os.remove(intermediate_file)


def compile_to_intermediate(input_file, intermediate_file):
    res = subprocess.run(['eosio-cpp', '-O0', '-c', input_file, '-o', intermediate_file], capture_output=False)
    res.check_returncode()


def link_wasm(intermediate_file, wasm_output_file):
    res = subprocess.run(['eosio-ld', intermediate_file, '-o', wasm_output_file], capture_output=False)
    res.check_returncode()

if __name__ == "__main__":
    main()
