#!/bin/bash

for (( i = 0 ; i < 15 ; i++))
do
   mkdir tfd$i
   cp run.sh_pasta tfd$i/run.sh
   chmod +x tfd$i/run.sh
   cd tfd$i
   ln -s ../visual_search visual_search
   cd ..
done

