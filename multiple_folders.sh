#!/bin/bash

while read line; do

  ./process_folder_bi.sh "${line}" 1500

done < folders.txt
