#!/bin/bash
cd code
make
cd ..
time ./runPublic
echo "public-block1.out  "
diff myoutputs1/public-block1.log outputs1/public-block1.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi
echo "public-assoc1.out  "
diff myoutputs1/public-assoc1.log outputs1/public-assoc1.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi
echo "public-write1.out  "
diff myoutputs1/public-write1.log outputs1/public-write1.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi
echo "public-instr1.out  "
diff myoutputs1/public-instr1.log outputs1/public-instr1.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi
echo "spice10.out  "
diff myoutputs1/spice10.log outputs1/spice10.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi
echo "spice100.out  "
diff myoutputs1/spice100.log outputs1/spice100.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi
echo "spice1000.out  "
diff myoutputs1/spice1000.log outputs1/spice1000.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi
echo "spice1.out"
diff myoutputs1/spice.log outputs1/spice1.out
ret=$?

if [[ $ret -eq 0 ]]; then
    echo "passed--------------"
else
    echo "failed--------------"
fi