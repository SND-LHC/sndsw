#!/bin/bash
brickIDs=({1..5}{1..4})
for ibrick in $(seq 0 19)
 do
  echo "processing brick ${brickIDs[ibrick]}"
  cd b0000${brickIDs[ibrick]}

   python $SNDSW_ROOT/shipLHC/scripts/fromsndsw2FEDRA/add_brickID_number.py ${brickIDs[ibrick]}

  cd ..
 done
