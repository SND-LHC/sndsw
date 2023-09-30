#!/bin/bash
#first argument signal file, second background file
brickIDs=({1..5}{1..4})
for ibrick in $(seq 0 19)
 do
  cd b0000${brickIDs[ibrick]}
  cp $SNDSW_ROOT/shipLHC/scripts/fromsndsw2FEDRA/csvconversion.py ./
  echo "now converting " b0000${brickIDs[ibrick]}
  python csvconversion.py ${brickIDs[ibrick]} ../$1 ../$2 #first parameter signal simulation, second background simulation
  cd ..
 done
