#!/bin/bash
#
# Script to find brightness measures of a folder

files=($1*)

# Data from the folder name
vazao=$(echo "$1" | awk -F _ '{print $5}')
vazao=${vazao:5:2}
massa=$(echo "$1" | awk -F _ '{print $2}')
massa=${massa:0:3}
angulo=$(echo "$1" | awk -F _ '{print $7}')
angulo=${angulo:0:(${#angulo}-1)}

saveFile="Results/WaveData_vazao_${vazao}_massa_${massa}_angulo_${angulo}.txt"
rm $saveFile
touch $saveFile
echo "Brightness in each height" >> $saveFile
cat "Files/brightness_help.txt" >> $saveFile
#echo "${vazao} , ${massa}"

# start and finnish frames number
start="${files[0]:(${#files[0]}-9):5}"
end="${files[-1]:(${#files[-1]}-9):5}"
echo "${files[0]:(${#files[0]}-9):5} , ${files[-1]:(${#files[-1]}-9):5}"

# iterate over 500 frames
n_iterations=$(echo "((${end} - ${start})/500)" | bc)

# loop
whileStart=${start}
i=1

while [[ $i -le $n_iterations ]]; do

  # calculate end
  whileEnd=$(echo "(${whileStart} + 500)" | bc)

  # join images
#  echo "${whileStart} , ${whileEnd}"
  echo "x" | ./joinImages -p "$1" -i "${whileStart}" -d 5 -l "${whileEnd}" -v $2 -a 0

  # process brightness information
  rm Files/brightness.txt
  ./waveMeasures


  # saves informations
  cat "Files/brightness.txt" >> $saveFile

  # iterate over the images
  whileStart=$whileEnd
  i=$(echo "${i} + 1" | bc)

done


