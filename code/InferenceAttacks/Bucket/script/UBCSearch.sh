#!/bin/zsh
for f in $(find ./ ! -iname "*.FSL" -type f)
do
python searchUBC.py $f >> ans
done
