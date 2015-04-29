#! /usr/bin/env python
# coding: utf-8

import sys
import os
from waflib import Options
from waflib import Logs

APPNAME = "mpaxos"
VERSION = "0.1"

top = "."
out = "bin"

pargs = ['--cflags', '--libs']
CFLAGS = []

COMPILER_LANG = "compiler_cxx"


def options(opt):
    opt.load(COMPILER_LANG)
    opt.load(['protoc'],
            tooldir=['waf-tools'])
    
    opt.add_option('-d', '--debug', dest='debug', default=False, action='store_true')
    opt.add_option('-l', '--log', dest="log", default='', help='log level', action='store')
    opt.add_option('-c', '--compiler', dest="compiler", default='', action="store")

def configure(conf):
#    conf.load("compiler_c++")
    conf.load(COMPILER_LANG)
    conf.load(['protoc'],
            tooldir=['waf-tools'])
    
#    _enable_pic(conf)
#    _enable_debug(conf)     #debug
#    _enable_log(conf)       #log level
#    _enable_static(conf)    #static
    _enable_cxx11(conf)

    conf.env.LIB_PTHREAD = 'pthread'
    conf.check_cfg(atleast_pkgconfig_version='0.0.0') 
#    conf.check_cfg(package='apr-1', uselib_store='APR', args=pargs)
#    conf.check_cfg(package='apr-util-1', uselib_store='APR-UTIL', args=pargs)
#    conf.check_cfg(package='json', uselib_store='JSON', args=pargs)
    conf.check_cfg(package='protobuf', uselib_store='PROTOBUF', args=pargs)
    conf.check_cfg(package='libzmq', uselib_store='ZMQ', args=pargs)
#    conf.check_cfg(package='check', uselib_store='CHECK', args=pargs)
    conf.check_cfg(package='yaml-cpp', uselib_store='YAML-CPP', args=pargs)

    conf.env.PREFIX = "/usr"
    conf.env.LIBDIR = "/usr/lib"
    conf.env.INCLUDEDIR = "/usr/include"

def build(bld):

    bld.stlib(source=bld.path.ant_glob(['libmpaxos/proposer.cpp', 'libmpaxos/mpaxos.proto']), 
              target="mpaxos",
              includes="libmpaxos",
              use="PROTOBUF YAML-CPP ZMQ",
              install_path="${PREFIX}/lib")

    bld.program(source=['test/test_proposer.cpp'], 
                target="test_proposer.out", 
                includes="libmpaxos", 
                use="mpaxos", 
                install_path=False)

    bld.install_files('${PREFIX}/include', 
                      bld.path.ant_glob('include/mpaxos/*.hpp'))

    bld(features='subst', source='mpaxos.pc.in', 
        target = 'mpaxos.pc', encoding = 'utf8', 
        install_path = '${PREFIX}/lib/pkgconfig', 
        CFLAGS = ' '.join(CFLAGS), 
        VERSION="0.0", 
        PREFIX = bld.env.PREFIX)

def _enable_cxx11(conf):
    Logs.pprint("PINK", "C++11 features enabled")
    if sys.platform == "darwin":
        conf.env.append_value("CXXFLAGS", "-stdlib=libc++")
        conf.env.append_value("LINKFLAGS", "-stdlib=libc++")
    conf.env.append_value("CXXFLAGS", "-std=c++0x")
