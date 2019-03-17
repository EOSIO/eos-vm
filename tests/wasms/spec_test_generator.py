#! /usr/bin/python

import sys, re

spec_wast = open(sys.argv[1], "r")

start_wast_re = re.compile("START")
end_wast_re = re.compile("END")
assert_return = re.compile("\(assert_return")
assert_check = re.compile("\(assert_return \(invoke \".*\"\)\)")
assert_check_params = re.compile("\(assert_return \(invoke \".*\" \(i32.const.*\)\)\)")
assert_return_params = re.compile("\(assert_return \(invoke \".*\" \(i32.const.*\)\)")
i32_const = re.compile(".*i32.const.*\)\)")
f32_const = re.compile(".*f32.const.*\)\)")
i64_const = re.compile(".*i64.const.*\)\)")
f64_const = re.compile(".*f64.const.*\)\)")

_i32_const = re.compile(".*i32.const.*")
_f32_const = re.compile(".*f32.const.*")
_i64_const = re.compile(".*i64.const.*")
_f64_const = re.compile(".*f64.const.*")

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
    tmp = ""
    for line in spec_wast:
        if start_wast_re.match(line):
            break
        if assert_check_params.match(line):
            func_name = line.split()[2]
            sl = line.split()
            boost_test += "\nBOOST_CHECK(!bkend("+func_name+", "
            tmp = ""
            for i in range(3, len(sl)):
                if i % 2 == 0:
                    tmp += "(uint32_t)"+eat_paren(sl[i])+", "
            boost_test += tmp[:-2]+"));\n"
        elif assert_check.match(line):
            func_name = line.split()[2]
            boost_test += "\nBOOST_CHECK(!bkend("+func_name+";\n"
        elif assert_return_params.match(line):
            func_name = line.split()[2]
            sl = line.split()
            if i64_const.match(line):
                cast = "TO_UINT64"
                value = "(uint64_t)"+eat_paren(line.split()[-1])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+", "
                tmp = ""
                cast = ""
                for i in range(3, len(sl)-2):
                    if i % 2 == 0:
                        tmp += cast+eat_paren(sl[i])+", "
                        cast = ""
                    else:
                        if _i32_const.match(sl[i]):
                            cast = "(uint32_t)"
                        elif _i64_const.match(sl[i]):
                            cast = "(uint64_t)"
                        elif _f32_const.match(sl[i]):
                            cast = "(float)"
                        elif _f64_const.match(sl[i]):
                            cast = "(double)"

                boost_test += tmp[:-2]+")), "+value+");\n"
            elif f32_const.match(line):
                cast = "TO_F32"
                value = "(float)"+eat_paren(line.split()[-1])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+", "
                tmp = ""
                cast = ""
                for i in range(3, len(sl)-2):
                    if i % 2 == 0:
                        tmp += cast+eat_paren(sl[i])+", "
                        cast = ""
                    else:
                        if _i32_const.match(sl[i]):
                            cast = "(uint32_t)"
                        elif _i64_const.match(sl[i]):
                            cast = "(uint64_t)"
                        elif _f32_const.match(sl[i]):
                            cast = "(float)"
                        elif _f64_const.match(sl[i]):
                            cast = "(double)"

                boost_test += tmp[:-2]+")), "+value+");\n"
            elif f64_const.match(line):
                cast = "TO_F64"
                value = "(double)"+eat_paren(line.split()[-1])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+", "
                tmp = ""
                cast = ""
                for i in range(3, len(sl)-2):
                    if i % 2 == 0:
                        tmp += cast+eat_paren(sl[i])+", "
                        cast = ""
                    else:
                        if _i32_const.match(sl[i]):
                            cast = "(uint32_t)"
                        elif _i64_const.match(sl[i]):
                            cast = "(uint64_t)"
                        elif _f32_const.match(sl[i]):
                            cast = "(float)"
                        elif _f64_const.match(sl[i]):
                            cast = "(double)"
                boost_test += tmp[:-2]+")), "+value+");\n"

            elif i32_const.match(line):
                cast = "TO_UINT32"
                value = "(uint32_t)"+eat_paren(line.split()[-1])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+", "
                tmp = ""
                cast = ""
                for i in range(3, len(sl)-2):
                    if i % 2 == 0:
                        tmp += cast+eat_paren(sl[i])+", "
                        cast = ""
                    else:
                        if _i32_const.match(sl[i]):
                            cast = "(uint32_t)"
                        elif _i64_const.match(sl[i]):
                            cast = "(uint64_t)"
                        elif _f32_const.match(sl[i]):
                            cast = "(float)"
                        elif _f64_const.match(sl[i]):
                            cast = "(double)"
                boost_test += tmp[:-2]+")), "+value+");\n"
            else:
                boost_test += "\nBOOST_CHECK(!bkend("+func_name+"));\n"
        elif assert_return.match(line):
            func_name = eat_paren(line.split()[2])
            if i32_const.match(line):
                cast = "TO_UINT32"
                value = "(uint32_t)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+")), "+value+");\n"
            elif f32_const.match(line):
                cast = "TO_F32"
                value = "(float)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+")), "+value+");\n"
            elif i64_const.match(line):
                cast = "TO_UINT64"
                value = "(uint64_t)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+")), "+value+");\n"
            elif f64_const.match(line):
                cast = "TO_F64"
                value = "(double)"+eat_paren(line.split()[4])
                boost_test += "\nBOOST_CHECK_EQUAL("+cast+"(*bkend("+func_name+")), "+value+");\n"
            else:
                boost_test += "\nBOOST_CHECK(!bkend("+func_name+"));\n"
            

    return [test_wast, boost_test]

if len(sys.argv) > 2 and sys.argv[2] == 't':
    print get_test()[0]
else:
    print get_test()[1]
