#!/bin/bash
for i in {2..27}
do
   echo "PIN $i"
   let j=$i-1
   `gpio -g write $j 0`
   `gpio -g mode $i out`
   `gpio -g write $i 1`
   read
done