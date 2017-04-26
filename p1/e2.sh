#!/bin/bash
reset
cd code
make
cd ..
time ./runPublic2
diff myoutputs2/public-block.log outputs2/public-block.out
diff myoutputs2/public-assoc.log outputs2/public-assoc.out
diff myoutputs2/public-write.log outputs2/public-write.out
diff myoutputs2/public-instr.log outputs2/public-instr.out

diff myoutputs2/spice10000-assoc.log outputs2/spice10000-assoc.out
diff myoutputs2/spice10000-block.log outputs2/spice10000-block.out
diff myoutputs2/spice10000-instr.log outputs2/spice10000-instr.out
diff myoutputs2/spice10000-write.log outputs2/spice10000-write.out

# diff myoutputs2/spice10000-assoc.log outputs2/spice10.out
# diff myoutputs2/spice10000-block.log outputs2/spice100.out
# diff myoutputs2/spice10000-instr.log outputs2/spice1000.out
# diff myoutputs2/spice10000-write.log outputs2/spice1.out
