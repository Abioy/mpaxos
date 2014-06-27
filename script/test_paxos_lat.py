
import os
import subprocess
import yaml
import re
import sys


#EXE = "./bin/bench_mpaxos.out"
N_HOST = 5
DIR = ""
DIR_RESULT = "result.rspaxos.lat"
DIR_CONFIG = "config.beaker"
LABEL_DATA = ['1K', '4K', '16K', '64K', '256K', '1M', '4M', '16M']
HOSTS = []

def exe_command(command):
    print(command)
    subprocess.Popen(command, shell=True, executable='/bin/bash').wait()
    print("done")
    pass

def remkdir():
    exe_command("rm -rf %s" % DIR_RESULT)
    exe_command("mkdir %s" % DIR_RESULT)

def killall():
    for host in HOSTS:
        command = "ssh %s \"killall bench_mpaxos.out\"" % (host) 
        exe_command(command)
    pass

def lauch_test(n_send, n_group, sz_data, sz_coded):
    killall()
    # lauch slaves 
    for i in range(1, len(HOSTS)):
        host = HOSTS[i]
        print("lauching slave %d" % i)
        c0 = "%s/bin/bench_mpaxos.out -c %s/config/config.beaker/config.5.%d -s 0 -q 0" % (DIR, DIR, i+1) 
        command = "nohup ssh %s \"%s\" &" % (host, c0)
        exe_command(command)
        pass

    # lauch master
    host = HOSTS[0]
    i = 1
    command = "%s/bin/bench_mpaxos.out -c %s/config/config.beaker/config.5.%d -s %d -g %d -d %d -e %d -q 1 >& %s/script/%s/%d" % (DIR, DIR, i, n_send, n_group, sz_data, sz_coded, DIR, DIR_RESULT, sz_data + sz_coded)
    #command = "%s/bin/bench_mpaxos.out -c %s/config/config.beaker/config.5.%d -s %d -g %d -d %d -e %d -q 1" % (DIR, DIR, i, n_send, n_group, sz_data, sz_coded)
    exe_command(command)
    killall()

if __name__ == "__main__":
    stream = open("env_beaker.yaml", "r")    
    yv = yaml.load(stream)
    HOSTS = yv['host']
    DIR = yv['dir']

    remkdir()

    base = 1024
    for i in range(8):
        lauch_test(10, 1, base, 0)
        base *= 4

    # collect results
    base = 1024
    lats = []
    for i in range(8):
        f = open("%s/%d" % (DIR_RESULT, base))

        ratesum = 0.0
        rate=0.0
        for line in f.readlines():
            r = re.match(r".+ (\d+) proposals commited in (\d+)ms.+", line)
            if r:
                c = int(r.group(1))
                t = int(r.group(2))
                lat = t * 1.0 / c 
                lats.append(lat)
#                rate = c / (t / 1000.0)
#                rates[j].append(rate)
#                ratesum += rate
#        sys.stdout.write("\t%f" % rate)
        base *= 4

    print("-------------------------------------------------")
    print("-----------------Paxos Latency (ms)--------------")
    print("-------------------------------------------------")
    
    for data in LABEL_DATA:
        sys.stdout.write("%s\t" % data)
    sys.stdout.write("\n")

    for lat in lats:
        sys.stdout.write("%s\t" % lat)
    sys.stdout.write("\n")


