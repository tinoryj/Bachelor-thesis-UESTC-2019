#!/bin/bash

# path of fsl trace
fsl='../data/fsl/2013'

# users considered in backups
users=('004' '007' '012' '013' '015' '028')

# auxiliary information
date_of_aux=('2013-01-22' '2013-01-29' '2013-02-05')

hasher_outputs='tmp'

# count prior backups (as auxiliary information) and launch frequency analysis
for aux in ${date_of_aux[@]}; do
        for user in ${users[@]}; do
                snapshot="fslhomes-user${user}-${aux}"
                if [ -f "${fsl}"/${snapshot}.tar.gz ]; then
                        tar zxf "${fsl}"/${snapshot}.tar.gz
                        fs-hasher/hf-stat -h ${snapshot}/${snapshot}.8kb.hash.anon > tmp/${snapshot}
                        ./encryptFSL tmp/${snapshot}
                        ./readChunksToDBs tmp/${snapshot}.enc 0 cdbs/F_${aux} cdbs/B_${aux}
                        ./readChunksToDBs tmp/${snapshot} 1 mdbs/F_${aux} mdbs/B_${aux}
                        rm -rf ${snapshot}
                        rm -rf tmp/${snapshot}
                        rm -rf tmp/${snapshot}.enc
                fi
        done
        mv cdbs/* /home/tinoryj/db_data/cdbs/
        mv mdbs/* /home/tinoryj/db_data/mdbs/
done
