ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -Wall
AM_CFLAGS   =
AM_CXXFLAGS =

lib_LTLIBRARIES = librwlocks.la
librwlocks_la_SOURCES = \
./src/RWLockFavorNeither.cpp \
./src/RWLockFavorWriters.cpp \
./src/stdafx.cpp
librwlocks_la_LDFLAGS = -no-undefined

noinst_PROGRAMS = demo
demo_SOURCES = ./test/main.cpp
demo_LDADD = librwlocks.la

SUBDIRS = src
