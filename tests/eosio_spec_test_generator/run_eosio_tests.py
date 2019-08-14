#!/usr/bin/env python3
import os
import shutil
import subprocess
import sys

import compile_eosio_tests
import generate_eosio_tests

WASM_DIR = sys.argv[1]
EOSIO_DIR = sys.argv[2]
OUT_DIR = sys.argv[3]
GENERATOR_DIR = sys.argv[4]

generator = os.path.join(GENERATOR_DIR, 'eosio_test_generator')

test_failures = []

def main():
    json_files = sorted(list(filter(lambda x: x.find('json') > -1, os.listdir(WASM_DIR))))
    try:
        os.mkdir(OUT_DIR)
    except Exception:
        pass

    for j in json_files:
        print(j)
        dir_name = j.split('.')[0]
        new_dir = os.path.join(OUT_DIR, dir_name)
        json_file = os.path.join(WASM_DIR, j)

        cwd = os.getcwd()

        os.mkdir(new_dir)
        os.chdir(new_dir)

        out = subprocess.run([generator, json_file], capture_output=True)
        out.check_returncode()

        mkdirs(os.getcwd())
        copy(os.getcwd(), dir_name)
        compile_test(os.getcwd())
        generate_tests(os.getcwd())

        os.chdir(cwd)

    os.mkdir(os.path.join(OUT_DIR, '_FAILURES'))
    for tf in test_failures:
        name, out = tf
        out_file = os.path.join(OUT_DIR, '_FAILURES', name)
        with open(out_file, 'w') as f:
            f.write(out)

def mkdirs(directory):
    for f in os.listdir(directory):
        if f.find('cpp') > -1:
            a = f.split('.')
            b = a[1]
            try:
                os.mkdir(os.path.join(directory, b))
            except:
                pass
            os.rename(f, f'{b}/{f}')
        if f.find('map') > -1:
            a = f.split('.')
            b = a[1]
            try:
                os.mkdir(os.path.join(directory, b))
            except:
                pass
            os.rename(f, f'{b}/{f}')

def copy(directory, name):
    for d in os.listdir(directory):
        shutil.copy(
            os.path.join(WASM_DIR, f'{name}.{d}.wasm'),
            os.path.join(directory, d, 'test.wasm')
        )

def compile_test(directory):
    for d in os.listdir(directory):
        compile_eosio_tests.main(d, os.path.join(d, 'generated.wasm'), os.path.join(d, 'generated.map'))

def generate_tests(directory):
    for d in os.listdir(directory):
        g_wasm_file = os.path.join(d, 'generated.wasm')
        t_wasm_file = os.path.join(d, 'test.wasm')
        o_wast_file = os.path.join(d, 'wasm_spec.wast')
        map_file = os.path.join(d, 'generated.map')
        try:
            generate_eosio_tests.main(g_wasm_file, t_wasm_file, o_wast_file, map_file)
            compile_generated_wast(o_wast_file, os.path.join(directory, d))
            copy_new_test(os.path.join(directory, d, 'wasm_spec.wasm'))
            run_eosio_test(directory, d)
        except Exception as e:
            print(e)
            pass

def compile_generated_wast(wast_file, out_dir):
    out_file = os.path.join(out_dir, 'wasm_spec.wasm')
    out = subprocess.run(['eosio-wast2wasm', wast_file, '-o', out_file], capture_output=True)
    if out.returncode > 0:
        # TODO: Better messaging
        print('    ', wast_file, 'failed to compile')

def copy_new_test(wasm_file):
    test_dir = os.path.join(EOSIO_DIR, 'build/unittests/spec-contracts/wasm_spec')

    try:
        os.remove(os.path.join(test_dir, 'wasm_spec.wasm'))
    except Exception:
        pass

    shutil.copy(wasm_file, os.path.join(test_dir, 'wasm_spec.wasm'))

def run_eosio_test(directory, d):
    # cd into eos directory and run tests
    cwd = os.getcwd()

    os.chdir(os.path.join(EOSIO_DIR, 'build'))
    out = subprocess.run(
        ['./unittests/unit_test', '--run_test=api_tests/wasm_spec', '--', '--verbose'],
        capture_output=True
    )

    stderr = out.stderr.decode('utf-8')

    if (stderr.find('wasm_execution_error') > -1
            or stderr.find('eosio_assert_message_exception') > -1
            or stderr.find('wasm_serialization_error') > -1):
        name = directory.split('/')[-1]
        test_name = f'{name}.{d}'
        test_failures.append((test_name, stderr))

    os.chdir(cwd)

if __name__ == '__main__':
    main()
