#
# makefile for startw
#

CC = cl /nologo
CPP = $(CC)
CFLAGS = /O1 /W3
CPPFLAGS = $(CFLAGS)
LINK = link /nologo

!ifdef UNICODE
CPPFLAGS = $(CPPFLAGS) /DUNICODE /D_UNICODE
!endif


objs = startw.obj tnywmain.obj

all : startw.exe

startw.exe : $(objs)
	$(LINK) $** kernel32.lib user32.lib shell32.lib /out:$@ /opt:nowin98 /merge:.data=.text /merge:.rdata=.text /section:.text,erw

startw.obj : startw.cpp


# /Zl : NODEFAULTLIB
tnywmain.obj : tnywmain.c
	$(CC) $(CFLAGS) /Zl /c $*.c



clean :
	del $(objs)
	del startw.exe

