import os
import random
import sys
from datetime import datetime, date, time

tag = datetime.now().strftime("%m-%d-%H:%M")

if os.path.isdir('../results'):
    pass
else:
    os.mkdir('../results')

if os.path.isdir('../results/result_%s'%tag): 
    pass 
else: 
    os.mkdir('../results/result_%s'%tag)

node_num = 5
total_times = 10
all_times = 10
if len(sys.argv) > 1:
   node_num = sys.argv[1]
if len(sys.argv) > 2:
   total_times = sys.argv[2]
if len(sys.argv) > 3:
   all_times = int(sys.argv[3])

for i in range(all_times) :

    os.system('../bin/test_captain_random_thread %s %s >> ../results/result_%s/node%s_times%s_round%s'%(node_num, total_times, tag, node_num, total_times, all_times))


os.system('cat ../results/result_%s/node%s_times%s_round%s | grep ERROR'%(tag, node_num, total_times, all_times))    
pylog = open('pylog', 'a+')
dt = datetime.now()
pylog.write(dt.strftime("%A, %d. %B %Y %I:%M%p"));
pylog.write('\nnode_num\t')
pylog.write(str(node_num))
pylog.write('\ntotal_times\t')
pylog.write(str(total_times))
pylog.write('\nall_times\t')
pylog.write(str(all_times))
pylog.write('\n')
pylog.close()
