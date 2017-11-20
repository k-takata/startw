#
# makefile for startw
#

CC = cl /nologo
CPP = $(CC)
CFLAGS = /O1 /W3 /GF /FAsc /DUNICODE /D_UNICODE
CPPFLAGS = $(CFLAGS)
LINK = link /nologo
LDFLAGS = /merge:.rdata=.text

!ifndef TARGET_CPU
!if ("$(CPU)"=="AMD64" && !DEFINED(386)) || DEFINED(AMD64) || "$(PLATFORM)"=="x64" || "$(PLATFORM)"=="X64"
TARGET_CPU = x64
!elseif DEFINED(IA64)
TARGET_CPU = ia64
!else
TARGET_CPU = x86
!endif
!endif

!if "$(TARGET_CPU)"=="x86"
EXE_NAME = startw.exe
!else
EXE_NAME = startw64.exe
!endif


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

OBJDIR = obj$(TARGET_CPU)


objs = $(OBJDIR)\startw.obj $(OBJDIR)\tnywmain.obj $(OBJDIR)\startw.res
libs = kernel32.lib user32.lib shell32.lib shlwapi.lib

all: $(OBJDIR)\$(EXE_NAME)

$(OBJDIR):
	if not exist $(OBJDIR)\nul mkdir $(OBJDIR)

$(OBJDIR)\$(EXE_NAME): $(OBJDIR) $(objs)
	$(LINK) $(objs) $(libs) /out:$@ $(LDFLAGS) /stub:stub_60h.exe


$(OBJDIR)\startw.obj: startw.cpp
	$(CC) /Fo$@ $(CPPFLAGS) /Fa$*.cod /c startw.cpp

$(OBJDIR)\startw.res: startw.rc
	$(RC) /Fo$@ startw.rc


# /Zl : NODEFAULTLIB
$(OBJDIR)\tnywmain.obj: tnywmain.c
	$(CC) /Fo$@ $(CFLAGS) /Fa$*.cod /Zl /c tnywmain.c


clean:
	del $(objs)
	del $(OBJDIR)\startw*.exe



#stub_40h.exe : stub_40h.asm
#	lcc -o $@ -a $?
#	stbhdr -d $@

#stub_60h.exe : stub_60h.asm
#	lcc -o $@ -a $?
#	stbhdr -d $@
