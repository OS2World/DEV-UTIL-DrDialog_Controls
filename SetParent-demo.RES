� �� 0<   d   main-  k   p   q  bubble�    .  ,   /  0  ��� 0�  setValue�percent=ARG(1)

call main.bar1.Text percent||'#A bar set to '||percent||' %'
call main.bar2.Text 100-percent||'#Background color Red'
call main.bar3.Color '-', '#'||percent*2||' '||255-percent*2||' '||255-percent*2
call main.bar3.Text percent||'#Foregroundcolor white'
call main.bar4.Text percent||'#Different font: 18.Helv'
call main.bar5.Text percent||'#Copied File '||percent%10||' of 10'
call main.bar6.Text percent
call main.bar7.Text percent||'#Vertical'
call main.bar8.Text percent||'#'||percent||' %'init�DLLVERSION='drctl015'

parse source . . sourceFile
dllPath=FILESPEC('drive', sourcefile)||FILESPEC('path', sourcefile)||'drctl013.dll'
thedir=FILESPEC('drive', sourcefile)||FILESPEC('path', sourcefile)
theDir=LEFT(theDir,LENGTH(theDir)-1)
oldDir=DIRECTORY(theDir)
/* Register percent bar */
rc=RxFuncAdd("DRCtrlRegister", DLLVERSION, "DRCtrlRegister")
call DRCtrlRegister
rc=RxFuncAdd("DRCtrlVersion", DLLVERSION, "DRCtrlVersion")
rc=RxFuncAdd("DRCtrlPickDirectory", DLLVERSION, "DRCtrlPickDirectory")
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

�� �d 0    �  ��          �   � � �� d ���         �  � � L  -����        �   �  <  k ����        �  �U  �  p ����     �  �     - v  q ����Set parent demo    Main dialog Quit Copyright Chris Wohlgemuth 2003 DRD_BUBBLEHELP Bubblehelp control ���d 0V  �d Opencall d200.open
call d300.openInit;call hint "This dialog demonstrates the SetParent function"�q Init�call FONT "10.Helv"
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
call isDefault "D"�p Init0ret=text()

call text ret||' V'DRCtrlVersion()�k ClickEXITInitcall text '~Quit'����d 0K   )d This dialog demonstrates the new controlsp The copyrightk Quit button�� �� 0�   �   �  ��           J   �0 � � � � ��K        � O @�  �  .����     Click here to see how the focus changes with default DrDialog dialogs. The titlebar of the main dialog will be inactive after clicking. � �,0�  �  �  ��           h   � � � � ,��i        � m @� J � : /����       7 U@ �  � 3 0����     This dialogs parent was changed to the main dialog. Clicking here doesn't change the titlebar status. Furthermore this dialog moves accurately with the main dialog during dragging. Move the main dialog slowly to see the difference. The parent is set in the 'open' callback of the dialog. ���,0�   �,Open�/* The ID of the main dialog is 100  */
/* The ID of the child dialog is 300 */
call DemoDRCtrlSetParent 300, 100

rc=Position(430, 105)
call show�