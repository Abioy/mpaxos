import os
import random
import sys
from time import sleep
from datetime import datetime, date, time
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

    print (i)
    os.system('../bin/test_captain_thread %s %s %s'%(node_num, node_times, total_times))
