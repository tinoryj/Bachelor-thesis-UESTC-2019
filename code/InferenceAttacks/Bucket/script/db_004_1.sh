#!/bin/bash

# path of fsl trace
fsl='/home/tinoryj/data/fsl/2013'

# users considered in backups
users=('004')

# auxiliary information
date_of_aux=('2013-01-22' '2013-01-29' '2013-02-05' '2013-02-15' '2013-02-22' '2013-03-21' '2013-03-28')

hasher_outputs='tmp'

# count prior backups (as auxiliary information) and launch frequency analysis
for aux in ${date_of_aux[@]}; do
    snapshot="fslhomes-user${users}-${aux}"                
        if [ -f "${fsl}"/${snapshot}.tar.gz ]; then
            tar zxf "${fsl}"/${snapshot}.tar.gz
            fs-hasher/hf-stat -h ${snapshot}/${snapshot}.8kb.hash.anon > tmp/${snapshot}
            ./encryptFSL tmp/${snapshot}
            ./readChunksToDBs tmp/${snapshot}.enc  cdbs/F_${users}_${aux} cdbs/B_${users}_${aux}
            ./readChunksToDBs tmp/${snapshot}  mdbs/F_${users}_${aux} mdbs/B_${users}_${aux}
            rm -rf ${snapshot}
            rm -rf tmp/${snapshot}
            rm -rf tmp/${snapshot}.enc
        fi
        mv cdbs/F_${users}_${aux} /home/tinoryj/db_data/cdbs/
        mv cdbs/B_${users}_${aux} /home/tinoryj/db_data/cdbs/
        mv mdbs/F_${users}_${aux} /home/tinoryj/db_data/mdbs/
        mv mdbs/B_${users}_${aux} /home/tinoryj/db_data/mdbs/ 
done
