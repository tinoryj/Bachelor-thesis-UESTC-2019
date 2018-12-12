# Base Line
## Usage

### step 1

```
make 
```

### step 2

```
./encryptFSL FSL_H_filename
```

This step will creat the encrypted FSL_H file named "xxx.enc"

### step 3

using 

```
./readChunksToDBs /fsl-data.enc /cdbs/xxxFCDB /cdbs/xxxSFDB
./readChunksToDBs /fsl-data /mdbs/xxxFCDB /mdbs/xxxSFDB
```

### step 4

using 

```
./inferChunks /CFC /CSF /MFC /MSF threhold
```

to start infer step accroding to CFC CSF MCF MSF DBs
(couldn't run with step 3,[they using the same dbs])



# Case Study 1: file type

## Usage

### step 1

```
make 
```

### step 2

```
./encryptFSL FSL_H_filename
```

This step will creat the encrypted FSL_H file named "xxx.enc"


### step 3

```
python outPutFileData.py > typeOut.txt
```

Then, type in "FSL_F" file to get file type and chunk number message.


### step 4

using 

```
./readInChunksToDBsFileType /fsl-data.enc typeOut.txt /cdbs/xxxFCDB /cdbs/xxxSFDB /tdbs/xxxTDB
./readChunksToDBsFileType /fsl-data typeOut.txt /mdbs/xxxFCDB /mdbs/xxxSFDB /tdbs/xxxTDB
```

typeOut.txt is made by step 3.

### step 5

using 

```
./inferChunksType /CFC /CSF /MFC /MSF /OUTPUT TYPENAME
```

`TYPENAME` is the type need to infer.
`/OUTPUT` is the ans db of correct infered chunk pair.


# Case Study II : acc in different file type

based on Case Study I.

## Usage

### Step 1:

```
./readChunksFile /xxxFSL /typeOut.txt /db
```

### Step 2:

```
./compareDB /db /OUTPUT
```






