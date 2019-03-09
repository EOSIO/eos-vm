#! /usr/bin/python

import sys, re

module_re = re.compile("\(module")
assert_re = re.compile("\(amodule")

infile = open(sys.argv[1], "r")

outstr = ""
otherstr = ""
index = 0
for line in infile:
    if assert_re.match(line):
        tmp = line.split()[-1][1:-2]
        outstr += tmp
        sl = str(len(tmp))
        otherstr += "{\n   binary_parser bp;\n   wasm_code_ptr cp{code.data()+"+str(index)+", "+sl+"};\n   module mod;\n   BOOST_CHECK_THROW(bp.parse_module(cp, "+sl+", mod), wasm_parser_exception);\n}\n"
    elif module_re.match(line):
        tmp = line.split()[-1][1:-2]
        outstr += tmp
        sl = str(len(tmp))
        otherstr += "{\n   binary_parser bp;\n   wasm_code_ptr cp{code.data()+"+str(index)+", "+sl+"};\n   module mod;\n   bp.parse_module(cp, "+sl+", mod);\n}\n"
        index += len(tmp)

print "static constexpr char* = \""+outstr+"\";"
print otherstr

