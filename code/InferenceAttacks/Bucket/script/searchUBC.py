import sys
import re

file = open(sys.argv[1]);

line = file.readline();
line = file.readline();
line = file.readline();
line = file.readline();
line = file.readline();
line = file.readline();
lineOut = line.replace('\n', '');
print sys.argv[1] + "\t" + lineOut;
file.close()
