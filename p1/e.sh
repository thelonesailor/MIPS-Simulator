#!/bin/bash
cd code
make
cd ..
time ./runPublic
diff myoutputs/public-block1.log outputs/public-block1.out
diff myoutputs/public-assoc1.log outputs/public-assoc1.out
diff myoutputs/public-write1.log outputs/public-write1.out
diff myoutputs/public-instr1.log outputs/public-instr1.out
diff myoutputs/spice10.log outputs/spice10.out
diff myoutputs/spice100.log outputs/spice100.out
diff myoutputs/spice1000.log outputs/spice1000.out
diff myoutputs/spice.log outputs/spice1.out
