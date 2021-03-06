� �� 0T   d   main�   �   z   y   {   x   �   w   }   �   |   � 	  k   p   ~   ��� 0  printHistogram�theHist="Brightness   Num pixel"||'0a'x

do a=1 to hist.0
	theHist=theHist||'0a'x||a||'          '||hist.a._grey
END
call main.c135.text theHist
returninit�
DLLVERSION='drctl015'

parse source . . sourceFile

thedir=FILESPEC('drive', sourcefile)||FILESPEC('path', sourcefile)
theDir=LEFT(theDir,LENGTH(theDir)-1)
oldDir=DIRECTORY(theDir)
/* Register new controls */
rc=RxFuncAdd("DRCtrlRegister", DLLVERSION , "DRCtrlRegister")
call DRCtrlRegister
rc=RxFuncAdd("DRCtrlVersion", DLLVERSION , "DRCtrlVersion")
rc=RxFuncAdd("DRCtrlPickDirectory", DLLVERSION , "DRCtrlPickDirectory")
rc=RxFuncAdd("DRCtrlGetHistogram", DLLVERSION , "DRCtrlGetHistogram")
rc=DIRECTORY(oldDir)

/*
The DLL must be in the current directory
or where the DrRexx.exe is.

The default DrDialog installation sets the working dir
for DrRexx.exe to the installation directory. So loading may
fail if you start your app with a double click. Copy
the DLL to your DrDialog installation directory or change
the working directory.
*/

�� �d 0�  �  �  ��          �  �, � �� d ���        �
 � ^ 
 � ����    	 	
    � r � P � ����         � [ N  z ����        1  � I P  y ����        A �_ \ V 
 { ����        I  �O & =  x ����       	 W �� � ] 	 � ����     a
 o   �� r � P w ����        z  �f I P  } ����        � �� ] = 
 � ����        �  �� & =  | ����      
   � �$ � � � ����        �  �  <  k ����       $ � �� 
 �  p ����        �  �  �� ~ ����Percent bar demo    Image DRD_IMAGE 0123456789 Current Selection: Query selection 0 0 0 0 Load image... Histogram DRD_HISTOGRAM 0123456789 Set Selection 0 0 Clear  Quit Copyright Chris Wohlgemuth 2001-2003 Image control demo area ���d 0p  �p Init0ret=text()

call text ret||' V'DRCtrlVersion()�k ClickEXITInitcall text '~Quit'�| Click.call main.c119.text ""
call main.c133.text ""�} Click�/* Image */
sel=main.c123.text()
call main.c133.text "#s "||sel

/* Histogram */
sel=main.c134.text()
call main.c119.text "#s "||sel
�x Click�rc=FilePrompt("*.*", "Load image file", ,"O")
if rc\="" THEN DO 
	call main.c119.color '+', '#255 0 0'
	call main.c119.color '-', '#0 0 255'
	call main.c119.text rc
	/* Enable selection */
	call main.c119.text "#e 1"
	call main.c133.text rc
	/* Enable selection */
	call main.c133.text "#e 1"
	rc=DRCtrlGetHistogram(rc, 'hist.')
	call printHistogram
END

rc=main.c133.text()
call main.c123.text rc

rc=main.c119.text()
call main.c134.text rc
�y Click`
rc=main.c133.text()
call main.c123.text rc

rc=main.c119.text()
call main.c134.text rc

����d 0�   )d This dialog demonstrates the new controlsp The copyrightk Quit button| Clear imagew Use mouse for selectionx Load an image file,y Query the selection rectangle from the image� Use mouse for selection�