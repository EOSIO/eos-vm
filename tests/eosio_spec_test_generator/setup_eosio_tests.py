#!/usr/bin/env python3
from multiprocessing import Pool

import os
import shutil
import subprocess
import sys

import compile_eosio_tests
import generate_eosio_tests

WASM_DIR = ''
EOSIO_DIR = ''
OUT_DIR = ''
GENERATOR_DIR = ''

generator = ''

test_failures = []

def main():
    json_files = sorted(list(filter(lambda x: x.find('json') > -1, os.listdir(WASM_DIR))))
    try:
        os.mkdir(OUT_DIR)
    except Exception:
        pass

    for j in json_files:
        setup_tests(j)


def setup_tests(j):
    print(j)
    dir_name = j.split('.')[0]
    new_dir = os.path.join(OUT_DIR, dir_name)
    json_file = os.path.join(WASM_DIR, j)

    _cwd = os.getcwd()

    os.mkdir(new_dir)
    os.chdir(new_dir)

    out = subprocess.run([generator, json_file], capture_output=True)
    out.check_returncode()

    mkdirs()
    copy(dir_name)
    compile_wasm()
    generate_and_copy()

    os.chdir(_cwd)


def mkdirs():
    new_dirs = []
    for f in os.listdir():
        num = f.split('.')[1]
        new_dirs.append(num)

    for d in set(new_dirs):
        os.mkdir(d)

    for f in os.listdir():
        if not os.path.isdir(f):
            num = f.split('.')[1]
            os.rename(f, os.path.join(num, f))


def copy(dir_name):
    for d in os.listdir():
        shutil.copy(
            os.path.join(WASM_DIR, f'{dir_name}.{d}.wasm'),
            os.path.join(os.getcwd(), d, 'test.wasm')
        )


def compile_wasm():
    cwd = os.getcwd()
    name = cwd.split('/')[-1]
    fs = os.listdir()

    fs_m = map(lambda x: (x, name), fs)

    # If there's a lot of files, break out and process in parallel.
    # Otherwise, we can just do serially.
    if len(fs) > 5:
        with Pool(os.cpu_count() - 2) as p:
            p.map(compile_eosio, fs_m)
    else:
        for d in fs_m:
            compile_eosio(d)

def compile_eosio(f):
    d, name = f
    compile_eosio_tests.main(
        d,
        f'{name}.{d}.wasm.cpp',
        f'{name}.{d}-int.wasm',
    )


def generate_and_copy():
    cwd = os.getcwd()
    name = cwd.split('/')[-1]
    for d in os.listdir():
        try:
            g_wasm_file = os.path.join(d, f'{name}.{d}-int.wasm')
            t_wasm_file = os.path.join(d, 'test.wasm')
            o_wast_file = os.path.join(d, f'{name}.{d}.wast')
            map_file = os.path.join(d, f'{name}.{d}.wasm.map')
            generate_eosio_tests.main(g_wasm_file, t_wasm_file, o_wast_file, map_file)
            wasm_file = f'{name}.{d}.wasm'
            out = subprocess.run(
                ['eosio-wast2wasm', o_wast_file, '-o', os.path.join(d, wasm_file)],
                capture_output=True
            )
            if out.returncode > 0:
                # TODO: Better messaging
                print('    ', o_wast_file, 'failed to compile')
        except Exception:
            continue

        test_dir = os.path.join(EOSIO_DIR, 'wasm_spec_tests')

        cpp_file = f'{name}.{d}.cpp'
        shutil.copy(os.path.join(d, cpp_file), os.path.join(test_dir, cpp_file))
        shutil.copy(os.path.join(d, wasm_file), os.path.join(test_dir, 'wasms', wasm_file))

if __name__ == '__main__':
    if len(sys.argv) < 5:
        print("""Please provide:
                Arg 1: Directory containing test wasms
                Arg 2: Directory containing eosio repo
                Arg 3: Directory for test files to be output to
                Arg 4: Build directory of the eosio_spec_test_generator

                ex:
                python setup_eosio_tests.py ~/code/eos-vm-test-wasms ~/code/eos /tmp ~/code/eos-vm/build/tests
              """)
        sys.exit(1)

    WASM_DIR = sys.argv[1]
    EOSIO_DIR = sys.argv[2]
    OUT_DIR = sys.argv[3]
    GENERATOR_DIR = sys.argv[4]

    generator = os.path.join(GENERATOR_DIR, 'eosio_test_generator')

    test_failures = []
    main()
