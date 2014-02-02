#
# makefile for startw
#

CC = cl /nologo
CPP = $(CC)
CFLAGS = /O1 /W3 /GF /FAsc
CPPFLAGS = $(CFLAGS)
LINK = link /nologo
LDFLAGS = /merge:.rdata=.text


# Get the version of cl.exe.
#  1. Write the version to a work file (mscver$(_NMAKE_VER).~).
!if ![(echo _MSC_VER>mscver$(_NMAKE_VER).c) && \
	(for /f %I in ('"$(CC) /EP mscver$(_NMAKE_VER).c 2>nul"') do @echo _MSC_VER=%I> mscver$(_NMAKE_VER).~)]
#  2. Include it.
!include mscver$(_NMAKE_VER).~
#  3. Clean up.
!if [del mscver$(_NMAKE_VER).~ mscver$(_NMAKE_VER).c]
!endif
!endif


!if $(_MSC_VER) < 1500
LDFLAGS = $(LDFLAGS) /opt:nowin98
!else
LDFLAGS = $(LDFLAGS) /dynamicbase:no
!endif

!if $(_MSC_VER) >= 1400
# Disable security checks to reduce the size.
CFLAGS = $(CFLAGS) /GS-
!endif


objs = startw.obj tnywmain.obj
objsw = startww.obj tnywmainw.obj startw.res
libs = kernel32.lib user32.lib shell32.lib shlwapi.lib

all : startw.exe startw9x.exe

startw.exe : $(objsw)
	$(LINK) $** $(libs) /out:$@ $(LDFLAGS) /stub:stub_60h.exe

startw9x.exe : $(objs)
	$(LINK) $** $(libs) /out:$@ $(LDFLAGS)


startw.obj : startw.cpp
	$(CC) /Fo$@ $(CPPFLAGS) /Fa$*.cod /c startw.cpp

startww.obj : startw.cpp
	$(CC) /Fo$@ $(CPPFLAGS) /Fa$*.cod /DUNICODE /D_UNICODE /c startw.cpp

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
