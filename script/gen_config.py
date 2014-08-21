import os;
import sys;
if len(sys.argv) > 1:
    step = int(sys.argv[1])
else:
    step = 1
for i in range(1,step+1):
    os.system('bin/gen_config %s'%i);
