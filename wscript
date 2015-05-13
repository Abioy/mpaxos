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
    opt.load('protoc unittest_gtest',
            tooldir=['waf-tools'])
    
    opt.add_option('-d', '--debug', dest='debug', default=False, action='store_true')
    opt.add_option('-l', '--log', dest="log", default='', help='log level', action='store')
    opt.add_option('-c', '--compiler', dest="compiler", default='', action="store")

def configure(conf):
    conf.env['CXX'] = "clang++"
#    conf.load("compiler_c++")
    conf.load(COMPILER_LANG)
    conf.load('protoc unittest_gtest')

    _enable_pic(conf)
    _enable_debug(conf)     #debug
    _enable_log(conf)       #log level
    _enable_static(conf)    #static
    _enable_cxx11(conf)


#    conf.check_cfg(package='apr-1', uselib_store='APR', args=pargs)
#    conf.check_cfg(package='apr-util-1', uselib_store='APR-UTIL', args=pargs)
#    conf.check_cfg(package='json', uselib_store='JSON', args=pargs)
#    conf.check_cfg(package='protobuf', uselib_store='PROTOBUF', args=pargs)
#    conf.check_cfg(package='libzmq', uselib_store='ZMQ', args=pargs)
#    conf.check_cfg(package='check', uselib_store='CHECK', args=pargs)
#    conf.check_cfg(package='yaml-cpp', uselib_store='YAML-CPP', args=pargs)

#    conf.env.LIB_PTHREAD = 'pthread'
    conf.env.PREFIX = "/usr"
    conf.env.LIBDIR = "/usr/lib"
    conf.env.INCLUDEDIR = "/usr/include"

def build(bld):

    bld.stlib(source=bld.path.ant_glob([ 'libmpaxos/view.cpp', 'libmpaxos/proposer.cpp', 'libmpaxos/acceptor.cpp', 
                                        'libmpaxos/captain.cpp', 'libmpaxos/commo.cpp', 'libmpaxos/mpaxos.proto']), 
              target="mpaxos",
              includes="libmpaxos",
              use="PROTOBUF",
              install_path="${PREFIX}/lib")

    bld.program(source=['test/test_proposer.cpp'], 
                target="test_proposer", 
                includes="libmpaxos", 
                use="mpaxos", 
                install_path=False)

    bld.program(source=['test/test_captain.cpp'], 
                target="test_captain", 
                includes="libmpaxos", 
                use="GTEST_PTHREAD mpaxos", 
                install_path=False)
    
    bld.program(features = 'gtest',
                source=['test/loli_gtest.cpp', 'libmpaxos/sample1.cc'], 
                target="loli_gtest", 
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

def _enable_debug(conf):
    if Options.options.debug:
        Logs.pprint("PINK", "Debug support enabled")
        conf.env.append_value("CFLAGS", "-Wall -Wno-unused -pthread -O0 -g -rdynamic -fno-omit-frame-pointer -fno-strict-aliasing".split())
        conf.env.append_value("CXXFLAGS", "-Wall -Wno-unused -pthread -O0 -g -rdynamic -fno-omit-frame-pointer -fno-strict-aliasing".split())
        conf.env.append_value("LINKFLAGS", "-Wall -Wno-unused -O0 -g -rdynamic -fno-omit-frame-pointer".split())
    else:
        conf.env.append_value("CFLAGS", "-Wall -O2 -pthread".split())
        conf.env.append_value("CXXFLAGS", "-Wall -O2 -pthread".split())
#        conf.env.append_value("LINKFLAGS", "-Wall -O2 -pthread".split())

    if os.getenv("CLANG") == "1":
        Logs.pprint("PINK", "Use clang as compiler")
        conf.env.append_value("C", "clang++")

def _enable_log(conf):
    if Options.options.log == 'debug':
        Logs.pprint("PINK", "Log level set to debug")
        conf.env.append_value("CFLAGS", "-DLOG_LEVEL=5")
        conf.env.append_value("CXXFLAGS", "-DLOG_LEVEL=5")
    elif Options.options.log == 'info':
        Logs.pprint("PINK", "Log level set to info")
        conf.env.append_value("CFLAGS", "-DLOG_LEVEL=4")
        conf.env.append_value("CXXFLAGS", "-DLOG_LEVEL=4")
    elif Options.options.log == '':
        pass
    else:
        Logs.pprint("PINK", "unsupported log level")
#    if os.getenv("DEBUG") == "1":


def _enable_static(conf):
    if os.getenv("STATIC") == "1":
        Logs.pprint("PINK", "statically link")
        conf.env.append_value("CFLAGS", "-static")
        conf.env.append_value("CXXFLAGS", "-static")
        conf.env.append_value("LINKFLAGS", "-static")
        pargs.append('--static')

def _enable_pic(conf):
    conf.env.append_value("CFLAGS", "-fPIC")
    conf.env.append_value("CXXFLAGS", "-fPIC")
    conf.env.append_value("LINKFLAGS", "-fPIC")

def _enable_cxx11(conf):
    Logs.pprint("PINK", "C++11 features enabled")
    if sys.platform == "darwin":
        conf.env.append_value("CXXFLAGS", "-stdlib=libc++")
        conf.env.append_value("LINKFLAGS", "-stdlib=libc++")
    conf.env.append_value("CXXFLAGS", "-std=c++0x")
