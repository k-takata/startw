#
# makefile for startw
#

CC = cl /nologo
CPP = $(CC)
CFLAGS = /O1 /W3 /GF
CPPFLAGS = $(CFLAGS)
LINK = link /nologo
LDFLAGS = /opt:nowin98 /merge:.rdata=.text


objs = startw.obj tnywmain.obj
objsw = startww.obj tnywmainw.obj startw.res
libs = kernel32.lib user32.lib shell32.lib shlwapi.lib

all : startw.exe startw9x.exe

startw.exe : $(objsw)
	$(LINK) $** $(libs) /out:$@ $(LDFLAGS) /stub:stub_60h.exe

startw9x.exe : $(objs)
	$(LINK) $** $(libs) /out:$@ $(LDFLAGS)


startw.obj : startw.cpp

startww.obj : startw.cpp
	$(CC) /Fo$@ $(CPPFLAGS) /DUNICODE /D_UNICODE /c startw.cpp

startw.res : startw.rc


# /Zl : NODEFAULTLIB
tnywmain.obj : tnywmain.c
	$(CC) $(CFLAGS) /Zl /c $*.c

tnywmainw.obj : tnywmain.c
	$(CC) /Fo$@ $(CFLAGS) /DUNICODE /D_UNICODE /Zl /c tnywmain.c


clean :
	del $(objs) $(objsw)
	del startw.exe startw9x.exe



#stub_40h.exe : stub_40h.asm
#	lcc -o $@ -a $?
#	stbhdr -d $@

#stub_60h.exe : stub_60h.asm
#	lcc -o $@ -a $?
#	stbhdr -d $@

