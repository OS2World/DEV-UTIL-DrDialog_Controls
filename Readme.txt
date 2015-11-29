User control for DrDialog V0.1.4 Aug 2003
--------------------------------------

This Rexx DLL contains new controls for use with DrDialog.
Use it by creating a user control when building your dialog
in DrDialog. Have a look at the file Demo.res for more
information.

The following controls are available:

-A percent bar (DRD_PERCENTBAR)
-A fly overhelp control (DRD_BUBBLEHELP)
-New with V01.1.2: An image control (DRD_IMAGE) which displays any
 image file supported by OS/2.
-New with V0.1.2: a directory picker
-New with V0.1.5: a histogram control showing the histogram of an image

Some functions are included to set the parent<->child relationship between
dialogs.


History:
-------

New with V0.1.5:

-Histogram control
-Function to get the histogram of an image

New with V0.1.4:

-Functions to set the parent of a dialog

New with V0.1.3: (not publicly available)

-Flyover help may have several lines now.


Rexx functions
--------------

The following Rexxfunctions are available in the DLL:

DRCtrlLoadFuncs:

	Load all functions of this DLL and make them available to Rexx.

DRCtrlDropFuncs:

	Drop all registered functions.

DRCtrlRegister:

	Register the new controls with DrDialog so they can be used.
	See 'How to use the controls' for more information.

DRCtrlVersion:

	Query the release.

	SAY DRCtrlVersion()
		returns "0.1.3" (or any other version number)

DRCtrlPickDirectory:

	Open a dialog to pick a directory

DRCtrlSetParent:

	Set the parent of a dialog. Normaly all dialogs created by DrDialog are
	childs of the desktop. You may set the owner of these dialogs using a built
	in function but not the parent.
	Why do you want to change the parent? For example if you create a dialog
	containing a lot of controls on top of another dialog the top dialog doesn't move
	with the bottom dialog when it is dragged using the mouse. If you click into the
	top dialog the focus switches to that dialog and the titlebar of the bottom
	dialog becomes inactive even if the top dialog doesn't has a titlebar. That is
	very confusing to the user. Using this new function it's possible to connect the
	top dialog to the bottom one eliminating these problems.

	Example:
		rc=DRCtrlSetParent( dialogID, newParentID)

	Returns: 
		1:	success
		0:	failure

	Note:
 		The dialogs MUST be main dialogs. This means they're
		childs of the desktop. This is true for all dialogs created by DrDialog
		If you want to change parents for other dialogs use
		DRCtrlSetParentFromHWND().
		If you change the parent of a dialog hints and bubblehelp no longer work for
		that dialog. 

DRCtrlGetHWND:

	Get the window handle of a dialog. This handle is used internally by PM.

	Example:
		rc=DRCtrlGetHWND( hwnd, ID)

	Returns:
		0:	failure
		other:	the window handle (HWND)

	Note:
		If hwnd==0 it's assumed the dialog is a child of the desktop. This
		is the case for dialogs initially created by DrDialog.

DRCtrlSetParentFromHWND:

	Sets the parent<->child relationship of dialogs using window handles.
	The dialogs may be child dialogs of a dialog.
	Use DRCtrlGetHWND() to query the HWND of a dialog.

	Example:
		rc=DRCtrlSetParentFromHWND( hwnd, newParentHWND)

	Returns: 
		1:	success
		0:	failure

	Note:
		If you change the parent of a dialog hints and bubblehelp no longer work for
		that dialog. 

DRCtrlGetHistogram:

	Gets the histogram of an image. 

	Example:
		rc=DRCtrlGetHistogram("x:\images\example.jpg", 'hist.')

	returns:
		1:	success
		0:	failure

	Note:
		The stem 'hist.' contains the histogram. hist.0 is 256 or 0 in
		case of an error. For each possible brightness level the number of
		pixels with that value is in hist.a._grey with a an index from 1
		to 256.

		DO a=1 to hist.0
			SAY hist.a._grey
		END

		This function can only be used with 24bit images.
	

How to use the controls
-----------------------
In your init function register the new Rexx function "DRCtrlRegister"
with Rexx and then call it to register the new controls with your DrDialog
application. The controls must be registered in every application because
this call is local to the calling process. 

NOTE:

The DLL must be in the current directory, the libpath
or where the DrRexx.exe is.

The default DrDialog installation sets the working dir
for DrRexx.exe to the installation directory. So loading may
fail if you start your app with a double click. Copy
the DLL to your DrDialog installation directory or change
the working directory then.

Example:
/***************************************/

/* Register the new function with Rexx */
/* This means make the function avaliable to Rexx */
rc=RxFuncAdd("DRCtrlRegister", 'drusrctl' , "DRCtrlRegister")

/* Register the new controls with your application */
/* This means make the controls available to DrDialog */
call DRCtrlRegister

...

/***************************************/


	
1. Percent bars
---------------
You may change the colors by using the Color() function of DrDialog: 

	/* Change color of bar */
	call Color '-','#255 0 0'
	/* Change color of text */
	call Color '+','#255 255 255'


You may change the font of the text by using the Font() function of DrDialog:

	/* Change font to 18.Helv */
	call Font '18.Helv'


The value of each bar may be set using the Text() function:

	/* Set bar to 30% */
	call Text '30'


The label of each bar may be set using the Text() function:
 
	/* Set bar to 30% and label to '3 of 10' */
	call Text '30#3 of 10'

The first part of the text string must be the value the bar should be set to.
Everything after the '#' is printed as the label on the bar. You may use
any text for the label or no label at all. If you don't want a label only
set the value.


2. Flyover help window
---------------------
Create a control of type DRD_BUBBLEHELP and set it as the default
for hints:

	call isDefault 'C'
	call isDefault 'D'

Now any hint specified for a control or dialog is shown as a fly over help
at the position of the mouse pointer.

Use the following calls to customize the fly over help.


You may change the colors by using the Color() function of DrDialog: 

	/* Change background color */
	call Color '-','#255 0 0'
	/* Change foreground color of text */
	call Color '+','#255 255 255'


You may change the font of the text by using the Font() function of DrDialog:

	/* Change font to 18.Helv */
	call Font '18.Helv'


Change the delay until the bubble opens using the Text() function:

	/* Change the delay to 300 ms untill the bubble is opened */
	call Text "#delay 300"


Change the time the bubble is shown:

	/* Show the bubble for 2500 ms */
	call Text "#show 2500"

If you want to have multiline hints use '0a'x to indicate a break.

	/* Show a two line hint */
	call hint "As you can see hints"||'0a'x||"may use several lines..."

Note:	DrDialog limits the length of a hint to 127 characters.


3. Image control
----------------

The image control displays any image file known to OS/2. Create a control
of type DRD_IMAGE and load an image on it using the text() function.

	call text "x:\the_path\image.jpg"

The image will be stretched to fit the control.

There's a function to select parts of the displayed image. A thin rectangle will
be drawn and the user may change and move the selection using the mouse. By using the
text function it's possible to set and query the current selection.

You have to enable this feature by sending the following control string to the control.

	/* Enable the select feature */
	call text "#E 1"

To disable the feature use the string 

	/* Disable the select feature */
	call text "#E 0"

Query the current selection:

	/* Get the current slected area */
	selection=text() 

The format of the returned string is:

	xLeft yBottom xRight yTop

If there's no selection the string will contain zeros:

	0 0 0 0 

To preset a selection area use the text function with a control string.

	call text "#s  xLeft yBottom xRight yTop"

	for example:

	call text "#s 12 22 100 145"


3. Histogram control
--------------------

The histogram control displays the histogram of an image file. Create a control
of type DRD_HISTOGRAM and load an image on it using the text() function.

	call text "x:\the_path\image.jpg"

The control should be 256 pixel wide an 200 pixel high. The height isn't critical
but if you use another width you may have troubles selecting areas of the histogram
because of rounding errors. The histogram will always be stretched to fit the control.

The drawing colors of the histogram may be set using the color() call of DrDialog.
Set the color before you load an image on the control.

There's a function to select a part of the displayed histogram. Two thin lines will
be drawn and the user may change and move the selection using the mouse. By using the
text function it's possible to set and query the current selection.

You have to enable this feature by sending the following control string to the control.

	/* Enable the select feature */
	call text "#E 1"

To disable the feature use the string 

	/* Disable the select feature */
	call text "#E 0"

Query the current selection:

	/* Get the current slected area */
	selection=text() 

The format of the returned string is:

	xLeft xRight

If there's no selection the string will contain zeros:

	0 0 

To preset a selection area use the text function with a control string.

	call text "#s  xLeft xRight"

	for example:

	call text "#s 12 100"

Note:
	The histogram control can only be used with 24bit images.
 	

5. Directory picker
-------------------

Use this REXX function to pick a directory.

	theDir=DRCtrlPickDirectory("x:\the_path\directory", "Title of directory")

When the first parameter is given, the directory dialog will be prefilled with the
given path. The second parameter may be omitted. The default Title is "Find directory".
The function returns the chosen directory or an empty string if the user selected cancel
or an error occurred.


License
-------
/*
 * Copyright (c) Chris Wohlgemuth 2001-2003 
 * All rights reserved.
 *
 * http://www.geocities.com/SiliconValley/Sector/5785/
 * http://www.os2world.com/cdwriting
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The authors name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */


Author
------
Copyright Chris Wohlgemuth 2001-2003

http://www.geocities.com/SiliconValley/Sector/5785/
http://www.os2world.com/cdwriting
