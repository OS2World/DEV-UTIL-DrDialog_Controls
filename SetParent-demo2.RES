� �� 0�   d   maing  1  k   p   q  bubble�   `  �   a  b  e  X   c  d  �    Y  Z  [  f  ,   \  ]  ^  _  ��� 0|  setValue�percent=ARG(1)

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
rc=RxFuncAdd("DemoDRCtrlSetParent", DLLVERSION , "DRCtrlSetParent")

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

�� �d 0�  �  �  ��          �   � ] �� d ���         � � � � g� ��      %   - �   � 1.��        J  �  <  k ����        O �U  �  p ����     o ~    ' Y 
 q ����Preferences     DRCtrlSetParent() demo ?                            14.Helvetica Bold Italic                 ����      ����Quit Copyright Chris Wohlgemuth 2003 DRD_BUBBLEHELP Bubblehelp control ���d 0�  �d Inits/* Open all settings pages invisible to prevent flickering */
DO a=200 to 600 by 100
	rc=openfor("d"||a)
END

�q Init�call FONT "10.Helv"
call FONT "9.WarpSans" 

call Color '-','#255 255 132'
call Color '+','#20 20 255'
/* Set the delay for the bubble */
call Text "#delay 0"
/* Set the time for showing the bubble */
call Text "#show 3000"

/* Set this control as the default for control hints */
call isDefault "C"

/* Set this control as the default for dialog hints */
call isDefault "D"
�p Init0ret=text()

call text ret||' V'DRCtrlVersion()�k ClickEXITInitcall text '~Quit'�1Select�call eventdata "data"

IF inserted='YES' THEN DO
SELECT
	WHEN data.2 = "+SELECT" THEN DO
		/* New item in the list selected */
		id=ITEM(data.1, "DATA")
		rc=showfor("d"||id)
	END
	WHEN data.2 = "-SELECT" THEN DO
		/* Item in the list deselected */
		id=ITEM(data.1, "DATA")
		rc=hidefor("d"||id)
	END
	OTHERWISE
		NOP
END

ENDInit�call view "Hierarchy", ""

rc=add("Settings 1",,,,200)
rc2=add("More settings",,,rc,300)
rc2=add("Even more settings...",,,rc,400)

rc=add("Settings 2",,,,500)
rc2=add("Different settings",,,rc,600)

inserted='YES'
����d 0   p The copyrightk Quit button�� ��0b   b   �  ��           J    � � � � ���K         O  � � � ! `����     Even more settings ����0�   ��Open~ thePos=main.c305.position()
parse var thepos x y cx rest

call position x+cx+20, y

call DemoDRCtrlSetParent id(), 100
�� ��0�   �   �  ��           �    � � � � ����        
 �  �
 � � # a����         �  � ; � K b����         �   �  � � e����     Settings 2   ����0�   ��Open~ thePos=main.c305.position()
parse var thepos x y cx rest

call position x+cx+20, y

call DemoDRCtrlSetParent id(), 100
�� �X0�   �   �  ��           h    � t � � X��i        
 m � V � & c����        x   �  � � d����     Last panel Different settings ���X0�   �XOpen~ thePos=main.c305.position()
parse var thepos x y cx rest

call position x+cx+20, y

call DemoDRCtrlSetParent id(), 100
�� �� 0�   �   �  ��           �    � � � � � ���        
 �  � � �  Y����        �  �
 � � 
 Z����        �  �
 � � 	 [����         � @ �  � j f����     Settings 1 Check Check  ���� 0�  �� Open~ thePos=main.c305.position()
parse var thepos x y cx rest

call position x+cx+20, y

call DemoDRCtrlSetParent id(), 100
�fInit�theText="This dialog demonstrates the use of the new function DRCtrlSetParent()"
theText=theText||" When selecting items in the tree control on the left a dialog is"
theText=theText||" shown on the right side on top of the main dialog. By changing the"
theText=theText||" parent of this small dialog from the desktop to the main dialog"
theText=theText||" it moves accurately with the main dialog and the focus isn't"
theText=theText||" taken from the main dialog so the titlebar remains active."

call text thetext
�� �,0�   �   �  ��           �    � � � � ,���         �  � � �  \����        �  � | b 
 ]����        �  � m b 
 ^����        �  � ^ b 	 _����     More settings Radio 1 Radio 2 Radio 3 ���,0�   �,Open~ thePos=main.c305.position()
parse var thepos x y cx rest

call position x+cx+20, y

call DemoDRCtrlSetParent id(), 100
�