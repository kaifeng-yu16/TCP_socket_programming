#! /bin/bash                                                                    
MACHINE=vcm-24502.vm.duke.edu
PORT=$1
NUM_PLAYERS=$2

sleep 1 
for ((i=1; i<=$NUM_PLAYERS; i++))
do
./player $MACHINE $PORT & 
done

wait
