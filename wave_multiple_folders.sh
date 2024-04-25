#!/bin/bash

while read line; do

  ./wave_process_folder.sh "${line}" 1500

done < folders.txt
