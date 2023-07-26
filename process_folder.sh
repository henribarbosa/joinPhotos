#!/bin/bash
#
# Script to find plug measures of a folder

files=($1*)

# Data from the folder name
vazao=$(echo "$1" | awk -F _ '{print $5}')
vazao=${vazao:5:2}
massa=$(echo "$1" | awk -F _ '{print $2}')
massa=${massa:0:3}

saveFile="PlugData_vazao_${vazao}_massa_${massa}.txt"
rm $saveFile
touch $saveFile
echo "Celerities (plug_length top_celerity bottom_celerity center_celerity) unit of Diameters and Diameters/s" >> $saveFile
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
  ./joinImages -p "$1" -i "${whileStart}" -d 2 -l "${whileEnd}" -v $2 -a 0

  # process plug information
  ./plugMeasures

  # saves informations
  number_lines=$(wc -l "Files/celerities.txt" | awk '{print $1}')
  number_lines=$(echo "$number_lines - 1" | bc) # skip header
  #echo "${number_lines}"
  tail -n "${number_lines}" "Files/celerities.txt" >> ${saveFile}

  # iterate over the images
  whileStart=$whileEnd
  i=$(echo "${i} + 1" | bc)

done


