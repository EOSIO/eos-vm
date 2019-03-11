#! /usr/bin/python

import sys, re

spec_wast = open(sys.argv[1], "r")

start_wast_re = re.compile("START")
end_wast_re = re.compile("END")
assert_return = re.compile("\(assert_return")
i32_const = re.compile(".*i32.const.*")
f32_const = re.compile(".*f32.const.*")
i64_const = re.compile(".*i64.const.*")
f64_const = re.compile(".*f64.const.*")

def eat_paren(s):
    i = 1
    for n in range(0,len(s)):
        if s[-i] != ')':
            break
        i += 1
    return s[:-(i-1)]

def get_test():
    test_wast = "" 
    test_wast_mode = False
    for line in spec_wast:
        if end_wast_re.match(line):
            test_wast_mode = False
            break
        if test_wast_mode:
            test_wast += line
        if start_wast_re.match(line):
            test_wast_mode = True
    
    boost_test = ""
    cast = ""
    value = ""
    for line in spec_wast:
        if start_wast_re.match(line):
            break
        if assert_return.match(line):
            func_name = eat_paren(line.split()[2])
            if i32_const.match(line):
                cast = "TO_UINT32"
                value = "(uint32_t)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*ctx.execute("+func_name+")), "+value+");\n"
            elif f32_const.match(line):
                cast = "TO_F32"
                value = "(float)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*ctx.execute("+func_name+")), "+value+");\n"
            elif i64_const.match(line):
                cast = "TO_UINT64"
                value = "(uint64_t)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*ctx.execute("+func_name+")), "+value+");\n"
            elif f64_const.match(line):
                cast = "TO_F64"
                value = "(double)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*ctx.execute("+func_name+")), "+value+");\n"
            else:
                boost_test += "\nBOOST_CHECK_EQUAL(ctx.execute("+func_name+"), false);\n"
            

    return [test_wast, boost_test]

if len(sys.argv) > 2 and sys.argv[2] == 't':
    print get_test()[0]
else:
    print get_test()[1]
