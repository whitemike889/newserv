INCPATH=../../
include ../../build.mk
include ../build.mk

.PHONY: all
all: chanserv_newsearch.so

chanserv_newsearch.so: chanserv_newsearch.o formats.o csns-qusers.o interface.o csns-qlasthost.o csns-qemail.o csns-qsuspendreason.o csns-qusername.o csns-qchanflags.o csns-qsuspended.o
