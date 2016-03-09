#!/bin/csh

foreach NUM_MAQUINAS ( 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 )

cp visual_search_0.cml tfd${NUM_MAQUINAS}
cd tfd${NUM_MAQUINAS}
sed -r "s/NUM_OFFSET = (.?);/NUM_OFFSET = $NUM_MAQUINAS;/"  ../visual_search_0.cml > visual_search_0.cml
cd ..
end


