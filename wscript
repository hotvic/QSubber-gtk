#! /bin/env python
# encoding: utf-8

top = '.'
out = 'build'

VERSION = '0.1'
APPNAME = 'QSubber'

def options(opt):
    opt.load('compiler_c vala glib2')

def configure(conf):
    conf.load('compiler_c vala glib2')

    conf.check_cc(lib='m', uselib_store='M', mandatory=1)
    conf.check_cfg(package='gtk+-3.0', uselib_store='GTK3', atleast_version='3.10', mandatory=1, args='--cflags --libs')
    conf.check_cfg(package='libsoup-2.4', uselib_store='SOUP', atleast_version='2.42', mandatory=1, args='--cflags --libs')

    conf.recurse('src')

def build(bld):
    bld.load('compiler_c vala glib2')

    bld.recurse('src')

