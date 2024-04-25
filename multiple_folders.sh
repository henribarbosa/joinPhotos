#!/bin/bash

while read line; do

  ./process_folder.sh "${line}" 1500

done < folders.txt
