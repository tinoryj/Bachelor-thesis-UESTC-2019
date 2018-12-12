#!/bin/bash

# path of fsl trace
fsl='/home/tinoryj/data/fsl/2013'

# users considered in backups
user=('012')

# auxiliary information
aux=('2013-05-21')

hasher_outputs='tmp'

# count prior backups (as auxiliary information) and launch frequency analysis
for users in ${user[@]}; do
    snapshot="fslhomes-user${users}-${aux}"                
        if [ -f "${fsl}"/${snapshot}.tar.gz ]; then
            tar zxf "${fsl}"/${snapshot}.tar.gz
            fs-hasher/hf-stat -h ${snapshot}/${snapshot}.8kb.hash.anon > tmp/${snapshot}
            fs-hasher/hf-stat -f ${snapshot}/${snapshot}.8kb.hash.anon > tmp/${snapshot}.f
            python outPutFileData.py tmp/${snapshot}.f >> tmp/${snapshot}.fdata
            ./readChunksFile tmp/${snapshot} tmp/${snapshot}.fdata T_${users}_${aux}

            rm -rf ${snapshot}
            rm -rf tmp/${snapshot}

            rm -rf tmp/${snapshot}.fdata
            rm -rf tmp/${snapshot}.f
            mv T_${users}_${aux} /home/tinoryj/db_data/case/dbs/
        fi
done
