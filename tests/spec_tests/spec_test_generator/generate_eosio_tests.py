#!/usr/bin/env python3

import re
import subprocess
import sys

class LookAhead():
    """
    Look ahead iterator. Look ahead using `.peek`.
    """
    _NONE = object()

    def __init__(self, iterable):
        self._it = iter(iterable)
        self._set_peek()

    def __iter__(self):
        return self

    def __next__(self):
        ret = self.peek
        self._set_peek()
        return ret

    def _set_peek(self):
        try:
            self.peek = next(self._it)
        except StopIteration:
            self.peek = self._NONE

    def __bool__(self):
        return self.peek is not self._NONE

class WASM:
    def __init__(self, types, imports, funcs, tables, memory, global_vars,
                 exports, data, elems, max_type=None):
        self.types = types
        self.imports = imports
        self.funcs = funcs
        self.tables = tables
        self.memory = memory
        self.global_vars = global_vars
        self.exports = exports
        self.data = data
        self.elems = elems
        self.max_type = max_type
        self.apply_func = None
        self.check_func = None
        self.apply_func_num = -1
        self.check_func_num = -1
        self.base_funcs = []

    def set_base_funcs(self):
        new_starting_index = 0
        for f in self.funcs:
            for l in f.split('\n'):
                match = re.search(func_regex, l)
                if match:
                    if int(match.group(2)) < 5:  # Test functions start at 5
                        self.base_funcs.append(f)
                        new_starting_index += 1

        self.funcs = self.funcs[new_starting_index:]


    def set_apply_func(self, apply_func = None):
        if apply_func:
            self.apply_func = apply_func
            return

        match = re.search(r'\(func \(;([0-9]+);\) \(type [0-9]+\) \(param i64 i64 i64\)', self.funcs[-1])
        if match:
            # Check last function for apply
            self.apply_func = self.funcs[-1]
            self.funcs = self.funcs[:-1]
            self.apply_func_num = match.group(1)

    def set_check_func(self, check_func = None):
        if check_func:
            self.check_func = check_func
            return

        match = re.search(r'\(func \(;([0-9]+);\) \(type [0-9]+\) \(param i32 i32\)', self.funcs[-1])
        if match:
            self.check_func = self.funcs[-1]
            self.funcs = self.funcs[:-1]
            self.check_func_num = match.group(1)


generated_max_type = 0
max_function_num = 0

function_symbol_table = {}

type_regex = r'(\(type \(;)([0-9]+)(.*)'
func_regex = r'(\(func \(;)([0-9]+)(;\) \(type )([0-9]+)(.*)'
call_regex = r'(call )([0-9]+)(.*)'
call_indirect_regex = r'(call_indirect \(type )([0-9]+)(.*)'
export_regex = r'(\(export "[\w\-\.]+" \(func )([0-9]+)(.*)'


def main():
    generated_wasm_file = sys.argv[1]
    test_wasm_file = sys.argv[2]
    out_wasm_file = sys.argv[3]

    generated_wasm = read_wasm(generated_wasm_file, False)
    global generated_max_type
    generated_max_type = generated_wasm.max_type

    # Remove the base, apply, and check functions from the funcs list
    # to make later substitution easier
    generated_wasm.set_base_funcs()
    generated_wasm.set_check_func()
    generated_wasm.set_apply_func()

    test_wasm = read_wasm(test_wasm_file, True)

    test_wasm, type_map = shift_types(test_wasm, generated_wasm.max_type)
    test_wasm = shift_funcs(test_wasm, type_map)
    test_wasm = shift_exports(test_wasm)

    generated_wasm = shift_generated_wasm(generated_wasm)

    build_symbol_table(generated_wasm, test_wasm)

    generated_wasm = shift_apply_calls(generated_wasm)

    generated_wasm = shift_apply_export(generated_wasm)

    with open(out_wasm_file, 'w') as f:
        f.write(write_merged_wasm(generated_wasm, test_wasm))


def shift_apply_export(generated_wasm):
    exports = []
    for e in generated_wasm.exports:
        if re.search(export_regex, e).group(1).find('apply') > -1:
            if generated_wasm.check_func:
                new_func_num = max_function_num - 1
            else:
                new_func_num = max_function_num

            n = re.sub(export_regex, lambda x: f'{x.group(1)}{new_func_num}{x.group(3)}', e)
            exports.append(n)
        else:
            exports.append(e)

    generated_wasm.exports = exports
    return generated_wasm


def shift_apply_calls(generated_wasm):
    global function_symbol_table

    lines = []
    for l in generated_wasm.apply_func.split('\n'):
        match = re.search(call_regex, l)
        if match:
            func_num = match.group(2)
            if int(func_num) > 4 and int(func_num) != generated_wasm.apply_func_num and int(func_num) != generated_wasm.check_func_num:
                new_check_num = function_symbol_table[match.group(2)]
                new_line = re.sub(call_regex, lambda x: f'{x.group(1)}{new_check_num}{x.group(3)}', l)
                lines.append(new_line)
            else:
                lines.append(l)
        else:
            lines.append(l)

    s = '\n'.join(lines)
    generated_wasm.apply_func = s

    return generated_wasm


def build_symbol_table(g_wasm, t_wasm):
    global function_symbol_table
    g_funcs = []
    t_funcs = []

    for f in g_wasm.funcs:
        for l in f.split('\n'):
            match = re.search(func_regex, l)
            if match:
                g_funcs.append(l)
    for e in t_wasm.exports:
        t_funcs.append(e)

    for i in range(0, len(t_funcs)):
        g_func = g_funcs[i]
        t_func = t_funcs[i]

        g_func_num = re.search(func_regex, g_func).group(2)
        t_func_num = re.search(export_regex, t_func).group(2)

        function_symbol_table[g_func_num] = t_func_num


def shift_generated_wasm(generated_wasm):
    def get_old_check_num(check):
        return re.search(func_regex, check).group(2)

    def shift_check_calls_in_apply(apply_func, old_check_num, new_check_num):
        new_apply = []
        for l in apply_func.split('\n'):
            match = re.search(call_regex, l)
            if match and match.group(2) == old_check_num:
                new_line = re.sub(call_regex, lambda x: f'{x.group(1)}{new_check_num}{x.group(3)}', l)
                new_apply.append(new_line)
            else:
                new_apply.append(l)

        return '\n'.join(new_apply)

    def shift_func(apply):
        global max_function_num

        new_func_num = int(max_function_num) + 1
        new_func = re.sub(func_regex, lambda x: f'{x.group(1)}{new_func_num}{x.group(3)}{x.group(4)}{x.group(5)}', apply)

        max_function_num = int(max_function_num) + 1

        return (new_func_num, new_func)

    apply = generated_wasm.apply_func
    check = generated_wasm.check_func

    new_apply_num, new_apply = shift_func(apply)

    if check:
        old_check_num = get_old_check_num(check)
        new_check_num, new_check = shift_func(check)
        shifted_new_apply = shift_check_calls_in_apply(new_apply, old_check_num, new_check_num)

        generated_wasm.check_func = new_check
        generated_wasm.check_func_num = new_check_num

        generated_wasm.apply_func = shifted_new_apply
        generated_wasm.apply_func_num = new_apply_num
    else:
        generated_wasm.apply_func = new_apply


    return generated_wasm


def write_merged_wasm(generated_wasm, test_wasm):
    global max_function_num

    out = '(module\n'
    for t in generated_wasm.types:
        out += t + '\n'
    for t in test_wasm.types:
        out += t + '\n'

    for i in generated_wasm.imports:
        out += i + '\n'
    for i in test_wasm.imports:
        out += i + '\n'

    for f in generated_wasm.base_funcs:
        out += f + '\n'

    for f in test_wasm.funcs:
        out += f + '\n'

    out += generated_wasm.apply_func + '\n'

    if generated_wasm.check_func:
        out += generated_wasm.check_func + '\n'

    # TODO:
    # How to handle tables? There can only be one
    #for t in generated_wasm.tables:
        #out += t + '\n'
    for t in test_wasm.tables:
        out += t + '\n'

    # TODO:
    # How to handle memory? There can only be one
    for m in generated_wasm.memory:
        out += m + '\n'
    #for m in test_wasm.memory:
        #out += m + '\n'

    for g in generated_wasm.global_vars:
        out += g + '\n'
    for g in test_wasm.global_vars:
        out += g + '\n'

    for e in test_wasm.exports:
        out += e + '\n'
    for e in generated_wasm.exports:
        out += e + '\n'

    for d in generated_wasm.data:
        out += d + '\n'
    for d in test_wasm.data:
        out += d + '\n'

    for e in test_wasm.elems:
        out += e + '\n'
    for e in generated_wasm.elems:
        out += e + '\n'

    out += ')\n'
    return out


def read_wasm(wasm, shift_calls=False):
    global generated_max_type
    out = subprocess.run(['eosio-wasm2wast', wasm], capture_output=True)
    generated_wast_string = out.stdout.decode('utf-8')

    lines = generated_wast_string.split('\n')
    last_line = lines[-2][0:-1] # strip the trailing parentheses off.
    new_lines = lines[0:-2]
    new_lines.extend([last_line, ''])
    la_lines = LookAhead(new_lines)

    next(la_lines)  # Skip the "module" line

    la_lines, types, max_type = get_types(la_lines)

    la_lines, imports = get_expr(la_lines, '(import ')
    la_lines, funcs = get_funcs(la_lines, shift_calls)
    la_lines, tables = get_expr(la_lines, '(table (;')
    la_lines, memory = get_expr(la_lines, '(memory (;')
    la_lines, global_vars = get_expr(la_lines, '(global (;')
    la_lines, exports = get_expr(la_lines, '(export ')
    la_lines, data = get_data(la_lines)
    la_lines, elems = get_expr(la_lines, '(elem ')

    if not shift_calls:
        generated_max_type = max_type

    return WASM(types, imports, funcs, tables, memory,
                global_vars, exports, data, elems, max_type)


def get_types(lines):
    max_type = 0
    types = []
    while re.search(type_regex, lines.peek):
        line = lines.peek
        max_type = get_type_number(line)
        types.append(line)
        next(lines)

    return (lines, types, max_type)


def get_type_number(type_line):
    match = re.search(type_regex, type_line)
    return match.group(2)


def get_funcs(lines, shift_calls):
    funcs = []
    while re.search(func_regex, lines.peek):
        func = lines.peek
        next(lines)
        while not re.search(func_regex, lines.peek):
            if non_func(lines.peek):
                break

            if shift_calls:
                if re.search(call_indirect_regex, lines.peek):
                    func += '\n' + shift_call_indirect(lines.peek)
                elif re.search(call_regex, lines.peek):
                    func += '\n' + shift_call(lines.peek)
                else:
                    func += '\n' + lines.peek
            else:
                func += '\n' + lines.peek
            next(lines)
        funcs.append(func)

    return (lines, funcs)


def get_expr(lines, expr):
    res = []
    while lines.peek.find(expr) > -1:
        res.append(lines.peek)
        next(lines)

    return (lines, res)


def get_data(lines):
    data = []
    while lines.peek.find('(data ') > -1:
        data.append(lines.peek)
        next(lines)

    return (lines, data)


def non_func(line):
    blacklist = ['table', 'memory', 'global']
    blacklist_2 = ['export', 'data', 'elem']
    for b in blacklist:
        if line.find(f'({b} (;') > -1:
            return True
    for b in blacklist_2:
        if line.find(f'({b} ') > -1:
            return True


def shift_call(line):
    match = re.search(call_regex, line)
    type_num = match.group(2)

    new_num = int(type_num) + 5

    new_call = re.sub(call_regex, lambda x: f'{x.group(1)}{new_num}{x.group(3)}', line)

    return new_call


def shift_call_indirect(line):
    match = re.search(call_indirect_regex, line)
    type_num = match.group(2)
    new_num = int(type_num) + int(generated_max_type) + 1

    new_call = re.sub(call_indirect_regex, lambda x: f'{x.group(1)}{new_num}{x.group(3)}', line)

    return new_call


def shift_types(wasm, max_type):
    types = wasm.types
    type_map = {}
    new_types = []
    for t in types:
        match = re.search(type_regex, t)

        type_num = match.group(2)
        new_type_num = int(type_num) + int(max_type) + 1
        type_map[type_num] = new_type_num

        new_type = re.sub(type_regex, lambda x: f'{x.group(1)}{new_type_num}{x.group(3)}', t)
        new_types.append(new_type)

    wasm.types = new_types
    return wasm, type_map


def shift_funcs(wasm, type_map):
    def get_type_num(f):
        return re.search(func_regex, f).group(4)

    def get_func_num(f):
        return re.search(func_regex, f).group(2)

    def inject_new_type_num(f, new_type_num):
        return re.sub(func_regex, lambda x: f'{x.group(1)}{x.group(2)}{x.group(3)}{new_type_num}{x.group(5)}', f)

    def inject_new_func_num(f, new_func_num):
        return re.sub(func_regex, lambda x: f'{x.group(1)}{new_func_num}{x.group(3)}{x.group(4)}{x.group(5)}', f)

    funcs = wasm.funcs
    new_funcs = []
    for f in funcs:
        type_num = get_type_num(f)
        new_type_num = type_map[type_num]

        func_num = get_func_num(f)
        new_func_num = int(func_num) + 5

        global max_function_num
        max_function_num = new_func_num

        new_func = inject_new_func_num(f, new_func_num)
        new_func = inject_new_type_num(new_func, new_type_num)

        new_funcs.append(new_func)

    wasm.funcs = new_funcs

    return wasm


def shift_exports(wasm):
    def get_func_num(f):
        return re.search(export_regex, f).group(2)

    def inject_new_func_num(f, new_func_num):
        return re.sub(export_regex, lambda x: f'{x.group(1)}{new_func_num}{x.group(3)}', f)

    exports = wasm.exports
    new_exports = []
    for e in exports:
        if e.find('(func ') > -1:
            func_num = get_func_num(e)
            new_func_num = int(func_num) + 5

            new_export = inject_new_func_num(e, new_func_num)

            new_exports.append(new_export)
        else:
            # TODO: Handle tables (and modules in general)
            new_exports.append(e)

    wasm.exports = new_exports

    return wasm


if __name__ == "__main__":
    main()
