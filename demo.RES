� �� 0�   d   maine  bar1f  bar2g  bar3i  bar4m  bar5o  bar8h  bar6j  bar7l   t   u   s   v   q  bubblew   z   y   x   |   {   }   k   p   �   �   n   r   ~      ��� 03  setValue�percent=ARG(1)

call main.bar1.Text percent||'#A bar set to '||percent||' %'
call main.bar2.Text 100-percent||'#Background color Red'
call main.bar3.Color '-', '#'||percent*2||' '||255-percent*2||' '||255-percent*2
call main.bar3.Text percent||'#Foregroundcolor white'
call main.bar4.Text percent||'#Different font: 18.Helv'
call main.bar5.Text percent||'#Copied File '||percent%10||' of 10'
call main.bar6.Text percent
call main.bar7.Text percent||'#Vertical'
call main.bar8.Text percent||'#'||percent||' %'init�DLLVERSION='drctl015'

parse source . . sourceFile
thedir=FILESPEC('drive', sourcefile)||FILESPEC('path', sourcefile)
theDir=LEFT(theDir,LENGTH(theDir)-1)
oldDir=DIRECTORY(theDir)
/* Register percent bar */
rc=RxFuncAdd("DRCtrlRegister", DLLVERSION , "DRCtrlRegister")
call DRCtrlRegister
rc=RxFuncAdd("DRCtrlVersion", DLLVERSION , "DRCtrlVersion")
rc=RxFuncAdd("DRCtrlPickDirectory", DLLVERSION , "DRCtrlPickDirectory")
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

�� �d 0g  g  �  ��          �  � � .� d ���     �  �  �	 � �  e ����     �  �   �	 � �  f ����     �  �   �	 � �  g ����     �  �   �	 } �  i ����     �  �   �	 m �  m ����     �     �	 U �  o ����          �� z  S h ����       &   �� z / S j ����        '  �� ] H  l ����       
 5�	 6 H  t ����        @ �	 $ H  u ����          O�S 6 , 
 s ����          P�S $ , 
 v ����     Q `   � . v  q ����    	 s
 }  �Ag � c w ����        � �CN Q  z ����        �  �C< B  y ����        �  ��� A  x ����        �  ��� A  | ����        � ��O t 
 { ����        �  ��< B  } ����        �  �  <  k ����        � �U  �  p ����         � �8 � 
 � ����        �  �9 ^  � ����          � R "� n ����        "  �  � 2 r ����        9  �17 � � ~ ����        Q  �1 � 4  ����Percent bar demo    DRD_PERCENTBAR  DRD_PERCENTBAR  DRD_PERCENTBAR  DRD_PERCENTBAR  DRD_PERCENTBAR  DRD_PERCENTBAR  DRD_PERCENTBAR  DRD_PERCENTBAR  Start demo... Delay (ms) Show for secs:   DRD_BUBBLEHELP Bubblehelp control DRD_IMAGE 0123456789 Current Selection: Query slection Load image... Clear 0 0 0 0 Set Selection Quit Copyright Chris Wohlgemuth 2001  Pick directory... Percent bar demo area Fly over help settings Image control demo area Directory picker demo ���d 01  �d Initxcall hint "This dialog demonstrates the new controls."||'0a'x||"As you can see hints"||'0a'x||"may use several lines..."�� ClickR
ret=DRCtrlPickDirectory( main.c128.text() ,"The Title")
call main.c128.text ret�� Initcall Range 256
�p Init0ret=text()

call text ret||' V'DRCtrlVersion()�k ClickEXITInitcall text '~Quit'�} Click8sel=main.c123.text()

call main.c119.text "#s "||sel
�| Clickcall main.c119.text ""�x Click�rc=FilePrompt("*.*", "Load image file", ,"O")
if rc\="" THEN DO 
	call main.c119.text rc
	/* Enable selection */
	call main.c119.text "#e 1"
END

rc=main.c119.text()

call main.c123.text rc

�y Click3
rc=main.c119.text()

call main.c123.text rc

�q Init�call FONT "10.Helv"
 call FONT "9.WarpSans" 

call Color '-','#255 255 132'
call Color '+','#20 20 255'
/* Set the delay for the bubble */
call Text "#delay 0"
/* Set the time for showing the bubble */
call Text "#show 1000"

/* Set this control as the default for control hints */
call isDefault "C"

/* Set this control as the default for dialog hints */
call isDefault "D"�v Done-call main.bubble.Text "#show "||select()*1000Initcall range 1,10�s DoneO/* Set the delay for the bubble */
call main.bubble.Text "#delay "||select()
Initcall range 0,2000�l Click5DO a=0 to 100
	call setValue a
	call sleep 40	
ENDInitcall Text "~Start demo..."�j Inithcall Color '-','#255 255 132'
call Color '+','#20 20 255'
call Font '9.Helv'
call text '20 #Vertical'�h Initcall Text '30'�o Initgcall Color '-','#255 255 132'
call Color '+','#255 0 0'
call Font '12.Courier'

call Text '0#0 %'
�m Init\call Color '-','#32 32 255'
call Color '+','#0 255 255'

call Text '0 #No Files copied'
�i Initzcall Color '-','#132 255 132'
call Color '+','#255 255 255'
call Font '18.Helv'
call text '47 #Different font: 18.Helv'�g Initccall Color '-','#255 0 0'
call Color '+','#255 255 255'

call Text '88 #Foregroundcolor white'
�f InitAcall Color '-','#255 0 0'
call Text '88 #Background color Red'
�e Init!call Text '12 #A bar set to 12 %'����d 0�  )d This dialog demonstrates the new controls� Insert a start directory herep The copyrightk Quit button| Clear imagex Load an image file,y Query the selection rectangle from the imagew Use mouse for selectionv Time for showing helps Changing the help delay.l Start the demoj Vertical percent bar 2h Vertical percent bar 1o Another font samplem Text samplei Font sampleg Foreground color samplef Background samplee Arbitrary text sample�