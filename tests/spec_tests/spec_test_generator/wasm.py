import re

from regexes import FUNC_REGEX, IMPORT_REGEX, TYPE_REGEX

from lookahead import LookAhead

class WASM(object):
    def __init__(self):
        self.types = []
        self.imports = []
        self.funcs = []
        self.tables = []
        self.memory = []
        self.global_vars = []
        self.exports = []
        self.data = []
        self.elems = []
        self.max_type = -1
        self.max_import = -1
        self.function_symbol_map = {}

    def read_wasm(self, wast_string):
        lines = wast_string.split('\n')
        last_line = lines[-2][0:-1] # strip the trailing parentheses off.
        new_lines = lines[0:-2]
        new_lines.extend([last_line, ''])
        la_lines = LookAhead(new_lines)

        next(la_lines)  # Skip the "module" line

        la_lines, types, max_type = self.get_types(la_lines)
        la_lines, imports, max_import = self.get_imports(la_lines)
        la_lines, funcs = self.get_funcs(la_lines)
        la_lines, tables = self.get_expr(la_lines, '(table (;')
        la_lines, memory = self.get_expr(la_lines, '(memory (;')
        la_lines, global_vars = self.get_expr(la_lines, '(global (;')
        la_lines, exports = self.get_expr(la_lines, '(export ')
        la_lines, data = self.get_data(la_lines)
        la_lines, elems = self.get_expr(la_lines, '(elem ')

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
        self.max_import = max_import

    def get_types(self, lines):
        max_type = 0
        types = []
        while re.search(TYPE_REGEX, lines.peek):
            line = lines.peek
            max_type = self.get_type_number(line)
            types.append(line)
            next(lines)

        return (lines, types, max_type)

    def get_type_number(self, type_line):
        match = re.search(TYPE_REGEX, type_line)
        return match.group(2)

    def get_imports(self, lines):
        max_import = 0
        imports = []
        while re.search(IMPORT_REGEX, lines.peek):
            line = lines.peek
            max_import = self.get_import_number(line)
            imports.append(line)
            next(lines)

        return (lines, imports, max_import)

    def get_import_number(self, import_line):
        match = re.search(IMPORT_REGEX, import_line)
        return match.group(6)

    def get_funcs(self, lines):
        funcs = []
        while re.search(FUNC_REGEX, lines.peek):
            func = lines.peek
            next(lines)
            while not re.search(FUNC_REGEX, lines.peek):
                if self.non_func(lines.peek):
                    break

                func += '\n' + lines.peek
                next(lines)
            funcs.append(func)

        return (lines, funcs)

    def get_expr(self, lines, expr):
        res = []
        while lines.peek.find(expr) > -1:
            res.append(lines.peek)
            next(lines)

        return (lines, res)

    def get_data(self, lines):
        data = []
        while lines.peek.find('(data ') > -1:
            data.append(lines.peek)
            next(lines)

        return (lines, data)

    def non_func(self, line):
        blacklist = ['table', 'memory', 'global']
        blacklist_2 = ['export', 'data', 'elem']
        for b in blacklist:
            if line.find(f'({b} (;') > -1:
                return True
        for b in blacklist_2:
            if line.find(f'({b} ') > -1:
                return True
