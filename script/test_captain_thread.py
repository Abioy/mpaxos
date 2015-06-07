import os
import random
import sys
from datetime import datetime, date, time
from time import sleep
tag = datetime.now().strftime("%m-%d-%H:%M")


if os.path.isdir('../results'):
    pass
else:
    os.mkdir('../results')

if os.path.isdir('../results/thread_result_%s'%tag): 
    pass 
else: 
    os.mkdir('../results/thread_result_%s'%tag)

node_num = 10
node_times = 10
total_times = 10
all_times = 10

if len(sys.argv) > 1:
   node_num = sys.argv[1]
if len(sys.argv) > 2:
   node_times = sys.argv[2]
if len(sys.argv) > 3:
   total_times = sys.argv[3]
if len(sys.argv) > 4:
   all_times = int(sys.argv[4])

for i in range(all_times) :

    os.system('../bin/test_captain_thread %s %s %s > ../results/thread_result_%s/node%s_nodetimes%s_times%s_round%s'%(node_num, node_times, total_times, tag, node_num, node_times, total_times, all_times))
    f=open('../results/thread_result_%s/node%s_nodetimes%s_times%s_round%s'%(tag, node_num, node_times, total_times, all_times))
    ff=f.read()
    if "ERROR" in ff:
        print ("test_captain_thread ERROR!!!!")
        pylog = open('pylog_thread', 'a+')
        dt = datetime.now()
        pylog.write(dt.strftime("%A, %d. %B %Y %I:%M%p"))
        pylog.write('\nnode_num\t')
        pylog.write(str(node_num))
        pylog.write('\nnode_times\t')
        pylog.write(str(node_times))
        pylog.write('\ntotal_times\t')
        pylog.write(str(total_times))
        pylog.write('\nall_times\t')
        pylog.write(str(all_times))
        pylog.write('\nthis_time\t')
        pylog.write(str(i))
        pylog.write('\n')
        pylog.close()
        break 
    else:
        print (i)
        print ("OK")
#        sleep(1)

#    os.system('cat ../results/thread_result_%s/node%s_times%s_round%s | grep ERROR'%(tag, node_num, total_times, all_times))    

