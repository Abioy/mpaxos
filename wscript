#! /usr/bin/env python
# coding: utf-8

import os
from waflib import Options
from waflib import Logs

APPNAME = "mpaxos"
VERSION = "0.1"

top = "."
out = "bin"

pargs = ['--cflags', '--libs']
CFLAGS = []

COMPILER_LANG = "compiler_c"
#COMPILER_LANG = "compiler_cxx"


def options(opt):
#    opt.load("compiler_c++")
    opt.load(COMPILER_LANG)
    
    opt.add_option('-d', '--debug', dest='debug', default=False, action='store_true')
    opt.add_option('-l', '--log', dest="log", default='', help='log level', action='store')
    opt.add_option('-c', '--compiler', dest="compiler", default='', action="store")

def configure(conf):
#    conf.load("compiler_c++")
    conf.load(COMPILER_LANG)
    
    _enable_pic(conf)
    _enable_debug(conf)     #debug
    _enable_log(conf)       #log level
    _enable_static(conf)    #static

    conf.env.LIB_PTHREAD = 'pthread'
    conf.check_cfg(atleast_pkgconfig_version='0.0.0') 
    conf.check_cfg(package='apr-1', uselib_store='APR', args=pargs)
    conf.check_cfg(package='apr-util-1', uselib_store='APR-UTIL', args=pargs)
    conf.check_cfg(package='json', uselib_store='JSON', args=pargs)
#    conf.check_cfg(package='libprotobuf-c', uselib_store='PROTOBUF', args=pargs)
#    conf.check_cfg(package='leveldb', uselib_store='LEVELDB', args=pargs)
    conf.check_cfg(package='check', uselib_store='CHECK', args=pargs)
#    conf.check(compiler='c', lib="leveldb", mandatory=True, uselib_store="LEVELDB")

    #c99
    conf.env.append_value("CFLAGS", "-std=c99")

    conf.env.PREFIX = "/usr"
    conf.env.LIBDIR = "/usr/lib"
    conf.env.INCLUDEDIR = "/usr/include"

    #leveldb
    #conf.env.append_value("CFLAGS", "-lleveldb")
    #conf.env.append_value("LINKFLAGS", "-lleveldb")

def build(bld):

    bld.stlib(source=bld.path.ant_glob("protobuf-c/*.c"), target="protobuf-c", includes="protobuf-c")
    bld.stlib(source=bld.path.ant_glob("libzfec/*.c"), target="zfec", includes="libzfec")
    bld.shlib(source=bld.path.ant_glob("libmpaxos/rpc/*.c libmpaxos/*.c"), target="mpaxos", includes="include libzfec protobuf-c libmpaxos", use="APR APR-UTIL JSON PTHREAD LEVELDB zfec protobuf-c", install_path="${PREFIX}/lib")
    bld.stlib(source=bld.path.ant_glob("libmpaxos/rpc/*.c libmpaxos/*.c"), target="mpaxos", includes="include libzfec protobuf-c libmpaxos", use="APR APR-UTIL JSON PTHREAD LEVELDB zfec protobuf-c", install_path="${PREFIX}/lib")
    bld.program(source="test/test_check.c", target="test_check.out", includes="include libmpaxos libzfec protobuf-c", use="mpaxos APR APR-UTIL CHECK zfec", install_path=False)
    bld.program(source="test/bench_mpaxos.c", target="bench_mpaxos.out", includes="include", use="mpaxos APR APR-UTIL", install_path=False)
    bld.program(source="test/bench_rpc.c", target="bench_rpc.out", includes="include libmpaxos protobuf-c libzfec", use="mpaxos APR APR-UTIL CHECK", install_path=False)

    bld.install_files('${PREFIX}/include', bld.path.ant_glob('include/mpaxos/*.h'))
    bld(features='subst', source='mpaxos.pc.in', target = 'mpaxos.pc', encoding = 'utf8', install_path = '${PREFIX}/lib/pkgconfig', CFLAGS = ' '.join( CFLAGS ), VERSION="0.0", PREFIX = bld.env.PREFIX )


def _enable_debug(conf):
    if Options.options.debug:
        Logs.pprint("PINK", "Debug support enabled")
        conf.env.append_value("CFLAGS", "-Wall -Wno-unused -pthread -O0 -g -rdynamic -fno-omit-frame-pointer -fno-strict-aliasing".split())
        conf.env.append_value("LINKFLAGS", "-Wall -Wno-unused -O0 -g -rdynamic -fno-omit-frame-pointer".split())
    else:
        conf.env.append_value("CFLAGS", "-Wall -O2 -pthread -DNDEBUG".split())

    if os.getenv("CLANG") == "1":
        Logs.pprint("PINK", "Use clang as compiler")
        conf.env.append_value("C", "clang++")

def _enable_log(conf):
    if Options.options.log == 'debug':
        Logs.pprint("PINK", "Log level set to debug")
        conf.env.append_value("CFLAGS", "-DLOG_LEVEL=5")
    elif Options.options.log == 'info':
        Logs.pprint("PINK", "Log level set to info")
        conf.env.append_value("CFLAGS", "-DLOG_LEVEL=4")
    elif Options.options.log == '':
        pass
    else:
        Logs.pprint("PINK", "unsupported log level")
#    if os.getenv("DEBUG") == "1":


def _enable_static(conf):
    if os.getenv("STATIC") == "1":
        Logs.pprint("PINK", "statically link")
        conf.env.append_value("CFLAGS", "-static")
        conf.env.append_value("LINKFLAGS", "-static")
        pargs.append('--static')

def _enable_pic(conf):
    conf.env.append_value("CFLAGS", "-fPIC")
    conf.env.append_value("CXXFLAGS", "-fPIC")
    conf.env.append_value("LINKFLAGS", "-fPIC")
