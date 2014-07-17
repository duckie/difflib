#!/usr/bin/env python
import difflib, sys

if len(sys.argv) < 3:
  sys.exit(1)

file_a = open(sys.argv[1],"r")
a = file_a.read()

file_b = open(sys.argv[2],"r")
b = file_b.read()

r = 0.
for i in range(0,10000):
  s = difflib.SequenceMatcher(None, a, b)
  r = s.ratio()

print(r) 
