#!/bin/bash

# path of fsl trace
dbsDir='/home/tinoryj/Documents/ProgsData/'

# users considered in backups
users=('004' '007' '012' '013' '015' '028')

fileType=('doc' 'docx' 'ppt' 'pptx' 'eml' 'jpg' 'png' 'img' 'amr' 'c' 'h' 'data' 'db' 'json' 'po' 'xml' 'pb' 'gz' 'lzma' 'pkg' 'bz2' 'html' 'log' 'lst' 'gmo' 'sh' 'out' 'txt' 'in' 'cfg' 'bin')

date="2013-05-21"

for user in ${users[@]}; do
    Tdb="T_${user}_${date}"
    Adb="ans${user}"
    for type in ${fileType}; do
        ./compareDB "${dbsDir}"/Tdb "${dbsDir}"/Adb ${type} > ans/ans_${user}_${type}
        #python insertXls.py  ans_${user}_${type}
    done
done

