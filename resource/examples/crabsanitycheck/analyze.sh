#!/bin/bash

GEO=`cat ~/.nuclearizer.cfg | grep Geo | awk -F">" '{ print $2 }' | awk -F"<" '{ print $1 }'`
GEO=${GEO/\(/\{}; 
GEO=${GEO/\)/\}};

if [[ ${GEO} == "" ]]; then
  echo "Unable to find geometry file. make sure you have set it in your .nuclearizer.cfg file"
  exit;
fi

nuclearizer -a -c Day_160612.nuclearizer.cfg -g ${GEO} &
nuclearizer -a -c Day_160613.nuclearizer.cfg -g ${GEO} &

wait

revan -a -f Day_160612.evta.gz -c Crab.revan.cfg -g ${GEO} &
revan -a -f Day_160613.evta.gz -c Crab.revan.cfg -g ${GEO} &

wait

mimrec -c Crab.mimrec.cfg -g ${GEO} -f Crab.tra -i -o Crab.png
mimrec -c Crab.mimrec.cfg -g ${GEO} -f Crab.tra -i -o Crab.C

