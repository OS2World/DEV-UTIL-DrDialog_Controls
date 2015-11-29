# Makefile for DRUSRCTL.DLL a DLL with a percent bar control
# and flyover help for DrDialog
#
# Compiled with VAC++ 3.08
# (other compilers should do the job after minor tweaking)

VERSION	     =	0_1_5
CC           = icc
CPPFLAGS     = /Ge- /Gm+ /G4 /Gl  /O+ /Oc+ /Ol- /Q+ $(VACDEBUG)

all:	drctl015.dll

drctl015.dll:	drusrctl.obj  drusrctl.def
	ILINK /NOFREE /NOE drusrctl.obj,drctl015.dll,,REXX mmPM2,drusrctl.def;
	rc drusrctl.rc drctl015.dll
	cp drctl015.dll ../

drusrctl.obj:	drusrctl.c
	icc -c -Ge- drusrctl.c
	
clean:
	-rm *.o
	-rm *.obj
	-rm *.dll
	-rm *.res

cleaner:
	-rm *.*~
	-rm *.flc

distribution:
	make
	-make clean
	-make cleaner
	-cd ../. && rm drdialog-controls-$(VERSION).zip
	-cd ../. && zip -r drdialog-controls-$(VERSION).zip *
