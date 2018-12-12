import sys
import re

file = open(sys.argv[1]);

print "Chunk Hash \t Chunk Size";
while 1:
    line = file.readline();
    if not line:
        break;
    if not re.search(r"^[a-z0-9]+:+[1-9]{3}", line):
        continue;
    chunk = line.split(":")[0];
    size = line.split(":")[1];
    sizeOut = size.replace('\n', '');
    print chunk[0:2] + ":" + chunk[2:4] + ":" + chunk[4:6] + ":" + chunk[6:8] + ":" + chunk[8:10] + ":00" + "\t" + sizeOut;
file.close()
