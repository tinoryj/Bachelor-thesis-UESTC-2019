import sys
import re

file = open(sys.argv[1]);

sum = 0;
while 1:
    line = file.readline()
    if not line:
        break
    strTmp = "Chunks";
    strTmp2 = "File path:";
    if not cmp(line[:10], strTmp2[:10]):
        #print line;
        if line.find('.') == -1:
            print 'NaNType';
        else:
            fileType = line.split('.');
            if not re.search(r"(\d{4}-\d{1,2}-\d{1,2})", fileType[len(fileType)-1]):
                print fileType[len(fileType)-1].replace('\n', '');
            else:
                print 'date'
    if not cmp(line[:6], strTmp[:6]):
        #print line;
        line = re.sub("\D", "", line);
        #print line;
        num = int(line);
        print num;
        sum = sum + num;

file.close()
