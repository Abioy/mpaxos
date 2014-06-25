#!/bin/bash

n_tosend=10

N_HOST=5
USER=shuai

MHOST[0]=none
MHOST[1]=beaker-20
MHOST[2]=beaker-21
MHOST[3]=beaker-22
MHOST[4]=beaker-23
MHOST[5]=beaker-24

TARGET=../bin/bench_mpaxos.out
DIR_RESULT=result.rspaxos.lat
is_exit=0
is_async=1
n_group=1
n_batch=1

mkdir $DIR_RESULT &> /dev/null

for i in $(seq $N_HOST)
do
    ssh $USER@${MHOST[$i]} "killall bench_mpaxos.out"
done


for sz_data in 1024 4096 16384 65536 262144 1048576 4194304 16777216
do
    echo "TESTING FOR $n_group GROUPS"

    
    for i in $(seq 2 $N_HOST)
    do
        echo "LAUNCHING DAEMON $i"
        group_begin=$(expr 100000 \* $i)
        command="nohup ~/bworkspace/mpaxos/bin/bench_mpaxos.out -c ~/bworkspace/mpaxos/config/config.beaker/config.$N_HOST.$i -s 0 -q 0 &> ~/bworkspace/mpaxos/script/result.rspaxos.lat/$sz_data.$i &" 
        echo $command
        nohup ssh $USER@${MHOST[$i]} $command &
    done

    for i in $(seq 1 1)
    do
    #    echo "LAUNCHING DAEMON $i"
    #    command="screen -m -d /bin/bash -c \"~/bworkspace/mpaxos/bin/bench_mpaxos.out -c ~/bworkspace/mpaxos/config/config.beaker/config.$N_HOST.$i -s $n_tosend -g $n_group -d 0 -e $sz_data -q 1 >& ~/bworkspace/mpaxos/script/result.rspaxos.lat/$sz_data.$i\"" 
    #    echo $command
    #    nohup ssh $USER@${MHOST[$i]} $command &
        ~/bworkspace/mpaxos/bin/bench_mpaxos.out -c ~/bworkspace/mpaxos/config/config.beaker/config.$N_HOST.$i -s $n_tosend -g $n_group -d 0 -e $sz_data -q 1 >& ~/bworkspace/mpaxos/script/result.rspaxos.lat/$sz_data.$i 
    done

    #for i in $(seq $N_HOST)
    #do
    #    r=""
    #    while [ "$r" = "" ]
    #    do
    #        command="cd ~/bworkspace/mpaxos/script/; cat result.rspaxos.lat/$sz_data.$i | grep \"All my task is done\""
    #        r=$(ssh $USER@${MHOST[$i]} "$command")
    #        sleep 1
    #    done
    #done
    
    # fetch all the results and kill mpaxos process
    for i in $(seq $N_HOST)
    do
        ssh $USER@${MHOST[$i]} "killall bench_mpaxos.out"
    done
done

