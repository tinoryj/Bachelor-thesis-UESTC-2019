import numpy as np;
import xlwt;
import xlrd;
import math;

users = ['004' ,'007' ,'012' ,'013' ,'015' ,'028'];

fileType = ['doc' ,'docx' ,'ppt' ,'pptx' ,'eml' ,'jpg' ,'png' ,'img' ,'amr' ,'c' ,'h' ,'cc' ,'cpp' ,'hpp' ,'java' ,'py' ,'m' ,'dll' ,'so' ,'a' ,'go' ,'js' ,'data' ,'db' ,'json' ,'po' ,'xml' ,'pb' ,'gz' ,'lzma' ,'pkg' ,'bz2' ,'zip' ,'tar' ,'rar' ,'7z' ,'html' ,'log' ,'lst' ,'gmo' ,'sh' ,'out' ,'txt' ,'in' ,'cfg' ,'bin' ,'vmdk'];

print (fileType);
workbook = xlwt.Workbook(encoding = 'utf-8');

for ID in range(6): 
    worksheet = workbook.add_sheet(users[ID]);
    print (worksheet);
    worksheet.write(0, 1, label = 'correct logic chunk');
    worksheet.write(0, 2, label = 'total logic chunk');
    worksheet.write(0, 3, label = 'correct unique chunks');
    worksheet.write(0, 4, label = 'total unique chunk');
    worksheet.write(0, 5, label = 'unique chunk over 10%');
    worksheet.write(0, 6, label = 'logic chunk over 10%');
    worksheet.write(0, 7, label = 'all chunk over 10%');
    worksheet.write(0, 8, label = 'file number');
    for file_type in range(len(fileType)):
        fileName = "ans_"+users[ID]+"_"+fileType[file_type];
        file = open(fileName);
        worksheet.write(file_type + 1, 0, label = fileType[file_type]);
        for i in range(8):
            line = file.readline()
            if not line:
                break
            worksheet.write(file_type + 1, i + 1, int(line));
        file.close();

workbook.save('task.xls')
        



