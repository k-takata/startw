#
# makefile for startw
#

CC = cl /nologo
CPP = $(CC)
CFLAGS = /O1 /W3 /GF
CPPFLAGS = $(CFLAGS) /MD
LINK = link /nologo


objs = startw.obj tnywmain.obj
objsw = startww.obj tnywmainw.obj

all : startw.exe startw9x.exe

startw.exe : $(objsw)
	$(LINK) $** kernel32.lib user32.lib shell32.lib /out:$@ /opt:nowin98 /merge:.rdata=.text

startw9x.exe : $(objs)
	$(LINK) $** kernel32.lib user32.lib shell32.lib /out:$@ /opt:nowin98 /merge:.rdata=.text


startw.obj : startw.cpp

startww.obj : startw.cpp
	$(CC) /Fo$@ $(CPPFLAGS) /DUNICODE /D_UNICODE /c startw.cpp


# /Zl : NODEFAULTLIB
tnywmain.obj : tnywmain.c
	$(CC) $(CFLAGS) /Zl /c $*.c

tnywmainw.obj : tnywmain.c
	$(CC) /Fo$@ $(CFLAGS) /DUNICODE /D_UNICODE /Zl /c tnywmain.c


clean :
	del $(objs) $(objsw)
	del startw.exe startw9x.exe

