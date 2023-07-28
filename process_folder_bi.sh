#!/bin/bash
#
# Script to find plug measures of a folder

files=($1*)

# Data from the folder name
vazao=$(echo "$1" | awk -F _ '{print $6}')
vazao=${vazao:5:2}
massa_v=$(echo "$1" | awk -F _ '{print $2}')
massa_v=${massa_v:0:3}
massa_z=$(echo "$1" | awk -F _ '{print $4}')
massa_z=${massa_z:0:3}
angulo=$(echo "$1" | awk -F _ '{print $8}')
angulo=${angulo:0:(${#angulo}-6)}

saveFile="Results/PlugData_vazao_${vazao}_massaVidro_${massa_v}_massaZirc_${massa_z}_angulo_${angulo}.txt"
#echo $saveFile
rm $saveFile
touch $saveFile
echo "Celerities (plug_length top_celerity bottom_celerity center_celerity) unit of Diameters and Diameters/s" >> $saveFile
#echo "${vazao} , ${massa}"

# start and finnish frames number
start="${files[0]:(${#files[0]}-9):5}"
end="${files[-1]:(${#files[-1]}-9):5}"
#echo "${files[0]:(${#files[0]}-9):5} , ${files[-1]:(${#files[-1]}-9):5}"

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
  ./joinImages -p "$1" -i "${whileStart}" -d 5 -l "${whileEnd}" -v $2 -a 0

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


