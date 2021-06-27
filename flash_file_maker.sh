#!/bin/bash

# Take in a desired bit file byte size, primary bit file and one or more slave fpga bit files and do the following:
#  
# Pad each one to the desired bit file byte size (currently hard coded to LFE5-85 max uncompressed bit file size of 2,293,750 bytes + 10 bytes to make a nicer alignment)
# Interleave the bytes of the non-primary bit files (i.e. second bit file byte 1:third bit file byte 1: ... :second bit file byte 2:third bit file byte 2:...)
# Append interleaved non-primary bit file to the end of the primary bit file to form the final programming file

args=("$@")

numArgs=${#args[*]}

if [ $numArgs -lt 2 -o $numArgs -gt 17 ] # at least one primary bit file and one secondary bit file; at most 1 primary and 16 secondary
then
    echo "INVALID NUMBER OF AGRUMENTS!"
    echo "USAGE:"
    echo "\tInputs: <primary bit file> <second bit file> [<third bit file> ...]"
    echo "\tOutput: <primary bit file>_merged.bit"
    exit -1
fi

desiredBytes=2293750
echo "Desired bit file byte size: $desiredBytes"; echo;

pFile=${args[0]}
echo "Primary bit file is $pFile"; echo;

# Zero pad primary file

fileBytes="$(du -hb $pFile | cut -f 1)"
padBytes=$(($desiredBytes-$fileBytes))

echo "Padding primary file $pFile with $padBytes bytes"; echo;
dd if=/dev/zero bs=1 count=$padBytes >> $pFile 
echo


declare -a myArray

# Loop through all bit files after the primary bit file.
# Zero pad each and convert to single column ASCII hex file.
for file in ${args[*]:1} 
do
    # Zero Pad
    fileBytes="$(du -hb $file | cut -f 1)"
    padBytes=$(($desiredBytes-$fileBytes))
    echo "Padding non-primary file $file with $padBytes bytes"; echo;
    dd if=/dev/zero bs=1 count=$padBytes >> $file 
    echo
    
    xxd -c 1 -ps $file $file.ascii

    myArray[${#myArray[*]}]=$file.ascii # save new name in a different array for later access

done

# Interleave the lines

paste -d "\n" ${myArray[*]} > interleaved.ascii

# Convert interleaved ascii hex text file back to a binary file

xxd -c 1 -ps -r interleaved.ascii interleaved.bit

# Append to end of padded primary file

cp -f $pFile download.bit

dd if=interleaved.bit bs=1 count="$(du -hb interleaved.bit | cut -f 1)" >> download.bit 
echo;

# Cleanup

echo "Cleaning up..."; echo;

for file in ${myArray[*]}
do
    rm -f $file
done

rm -f interleaved.ascii
rm -f interleaved.bit


echo "DONE!"; echo;

exit 0
