
import os
import yaml


#EXE = "./bin/bench_mpaxos.out"
N_HOST = 5
DIR = ""
HOSTS = []

def killall():
    for host in HOSTS:
        command = "ssh %s \"killall bench_mpaxos.out\"" % (host) 
        print(command)  
        os.system(command)
    pass

def lauch_test(n_send, n_group, sz_data, sz_coded):
    killall()
    # lauch slaves 
    for i in range(1, len(HOSTS)):
        host = HOSTS[i]
        print("lauching slave %d" % i)
        exe_command = "%s/bin/bench_mpaxos.out -c %s/config/config.beaker/config.5.%d -s 0 -q 0 &" % (DIR, DIR, i, DIR, i) 
        command = "ssh %s %s" % (host, exe_command)
        pass

    # lauch master
    host = HOSTS[0]
    command = "%s/bin/bench_mpaxos.out -c %s/config/config.beaker/config.5.%d -s %d -g %d -d %d -e %d -q 1 >& %s/script/result.rspaxos.lat/%d" % (DIR, DIR, i, n_send, n_group, sz_data, sz_coded, sz_data)
    os.system(command)
    killall()

if __name__ == "__main__":
    stream = open("env_beaker.yaml", "r")    
    yv = yaml.load(stream)
    HOSTS = yv['host']
    DIR = yv['dir']

    base = 1024
    for i in range(8):
        lauch_test(10, 1, 0, base)
        base *= 2

    # TODO analysis
