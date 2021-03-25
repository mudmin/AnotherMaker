#!/bin/bash
# install systbench with apt-install sysbench
# be sure to chmod +x test.sh so you can run the script
# run with
#    time ./test.sh

for f in {1..7}
do
        start=`date +%s`
        vcgencmd measure_temp
        sysbench --test=cpu --cpu-max-prime=25000 --num-threads=4 run >/dev/nul$
        end=`date +%s`
        runtime=$((end-start))
        echo "$runtime"
done
vcgencmd measure_temp
