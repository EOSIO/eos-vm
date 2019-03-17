#! /usr/bin/python

import sys
import re

export_re = re.compile(".*export \".*\"")
invoke_re = re.compile(".*invoke.*")

in_wasm = open(sys.argv[1], "r")

func_list = list()
for line in in_wasm:
    if export_re.match(line):
        m = export_re.search(line)
        func_list.append(m.group(0)[17:-1])
    if invoke_re.match(line):
        i = 0
        for f in func_list:
            if line.find(f) != -1:
                print (line.replace('"'+f+'"', str(i))[:-1])
                break
            i += 1
