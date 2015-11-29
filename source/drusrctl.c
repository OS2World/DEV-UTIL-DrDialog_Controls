/**********************************************************************
*                                                                     *
* DRUSRCTL.DLL                                                        *
*                                                                     *
* Copyright C. Wohlgemuth 2001-2003                                   *
*                                                                     *
* This Rexx DLL contains a new  controls for use with                 *
* DrDialog.                                                           *
*                                                                     *
**********************************************************************/
/*
 * Copyright (c) Chris Wohlgemuth 2001 
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



/* Include files */

#define  INCL_PM
#define  INCL_WINSYS
#define  INCL_DOS
#define  INCL_DOSMISC
#define  INCL_DOSNMPIPES
#define  INCL_ERRORS
#define  INCL_REXXSAA
#define  INCL_MMIOOS2
#define  INCL_WINTRACKRECT
#define  _DLL
#define  _MT
#include <os2.h>
#include <rexxsaa.h>
#include <malloc.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <conio.h>
#include "os2me.h"
#include "drusrctl.h"

#define MRFALSE (MRESULT)FALSE;
#define MRTRUE (MRESULT)TRUE;

#define DRDCTRL_VERSION         "0.1.5"
#define DRDCTRL_DLL_NAME        "DRCTL015"

/* Attention the first 4 bytes are used by DrDialog!! */
/* Window ULONG for the percent bar */
#define QWL_PERCENT 4 /* The location in the window words to store the percent value */
#define QWL_TEXTPTR 8 /* The ptr to our percent bar text */

/* Window ULONG for the bubble help control */
#define QWL_DELAY        4
/*#define QWL_TEXTPTR 8   already defined for percent bar */
#define QWL_SHOWTIME     12
#define QWL_SHOWBUBBLE   16
#define QWL_DELTAPOS     20
#define BUBBLEHELP_ULONGS 24

#define BUBBLE_DELAY_TIMER  1
#define BUBBLE_SHOW_TIMER   2
#define BUBBLE_CHECK_TIMER  3


#define QWL_HBITMAP         4  /* The location in the window words to store the bitmap handle */
#define QWL_TEXTPTR         8  /* The ptr to our selection text */
#define QWL_HAVESELECTION   12
#define QWL_PTSSTART        16
#define QWL_PTSTEMP         20
#define QWL_FLAGS           24 /* 0x00000001: enable select feature */

#define IMAGE_BYTES         28

#define xVal  12      /* x-distance of Bubble */
#define yVal  8      /* y-distance of Bubble */

/*********************************************************************/
/*  Declare all exported functions as REXX functions.                */
/*********************************************************************/

RexxFunctionHandler DRCtrlDropFuncs;
RexxFunctionHandler DRCtrlRegister;
RexxFunctionHandler DRCtrlLoadFuncs;
RexxFunctionHandler DRCtrlVersion;
RexxFunctionHandler DRCtrlPickDirectory;
RexxFunctionHandler DRCtrlSetParent;
RexxFunctionHandler DRCtrlGetHWND;
RexxFunctionHandler DRCtrlSetParentFromHWND;
RexxFunctionHandler DRCtrlGetHistogram;

/*********************************************************************/
/* RxFncTable                                                        */
/*   Array of names of the REXXUTIL functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/

static PSZ  RxFncTable[] =
   {
      "DRCtrlRegister",
      "DRCtrlLoadFuncs",
      "DRCtrlDropFuncs",
      "DRCtrlVersion",
      "DRCtrlPickDirectory",
      "DRCtrlSetParent",
      "DRCtrlGetHWND",
      "DRCtrlSetParentFromHWND",
      "DRCtrlGetHistogram",
   };

/*********************************************************************/
/* The handle of this function DLL                                   */
/*********************************************************************/
static HMODULE hModule=0;

PFNWP g_pfnwpOrgStaticProc=NULLHANDLE;
BOOL g_fDisableFlyOverTransparency=TRUE;
BOOL g_fDisableFlyOverShadow=TRUE;

/*********************************************************************/
/* Numeric Error Return Strings                                      */
/*********************************************************************/

#define  NO_UTIL_ERROR    "0"          /* No error whatsoever        */
#define  ERROR_NOMEM      "2"          /* Insufficient memory        */
#define  ERROR_FILEOPEN   "3"          /* Error opening text file    */

/*********************************************************************/
/* Alpha Numeric Return Strings                                      */
/*********************************************************************/

#define  ERROR_RETSTR   "ERROR:"
#define  EMPTY_RETSTR   ""

/*********************************************************************/
/* Numeric Return calls                                              */
/*********************************************************************/

#define  INVALID_ROUTINE 40            /* Raise Rexx error           */
#define  VALID_ROUTINE    0            /* Successful completion      */

/*********************************************************************/
/* Some useful macros                                                */
/*********************************************************************/

#define BUILDRXSTRING(t, s) { \
  strcpy((t)->strptr,(s));\
  (t)->strlength = strlen((s)); \
}



#define DEBUG_
#ifdef DEBUG
#define EXCEPTION_LOGFILE_NAME "j:\\arbeitsoberflaeche\\tst.log"
void HlpWriteToTrapLog(const char* chrFormat, ...)
{
  char logNameLocal[CCHMAXPATH];
  FILE *fHandle;

  sprintf(logNameLocal,"%s", EXCEPTION_LOGFILE_NAME);
  fHandle=fopen(logNameLocal,"a");
  if(fHandle) {


    va_list arg_ptr;
    void *tb;
    
    va_start (arg_ptr, chrFormat);
    vfprintf(fHandle, chrFormat, arg_ptr);
    va_end (arg_ptr);

    fclose(fHandle);
  }
}
#endif




HBITMAP loadBitmap (  PSZ pszFileName /*,PBITMAPINFOHEADER2 pBMPInfoHeader2*/)
{
    HBITMAP       hbm;
    HBITMAP       hbmTarget;
    MMIOINFO      mmioinfo;
        MMFORMATINFO  mmFormatInfo;
    HMMIO         hmmio;
        ULONG         ulImageHeaderLength;
        MMIMAGEHEADER mmImgHdr;
    ULONG         ulBytesRead;
    ULONG         dwNumRowBytes;
    PBYTE         pRowBuffer;
    ULONG         dwRowCount;
    SIZEL         ImageSize;
    ULONG         dwHeight, dwWidth;
    SHORT          wBitCount;
    FOURCC        fccStorageSystem;
    ULONG         dwPadBytes;
    ULONG         dwRowBits;
    ULONG         ulReturnCode;
    ULONG         dwReturnCode;
    HBITMAP       hbReturnCode;
    LONG          lReturnCode;
    FOURCC        fccIOProc;
    HDC           hdc;
    HPS           hps;
    HAB           hab;

    hab=WinQueryAnchorBlock(HWND_DESKTOP);

    ulReturnCode = mmioIdentifyFile ( pszFileName,
                                      0L,
                                      &mmFormatInfo,
                                      &fccStorageSystem,
                                      0L,
                                      0L);
    /*
     *  If this file was NOT identified, then this function won't
     *  work, so return an error by indicating an empty bitmap.
     */
    if ( ulReturnCode == MMIO_ERROR )
    {
            return (0L);
    }
    /*
     *  If mmioIdentifyFile did not find a custom-written IO proc which
     *  can understand the image file, then it will return the DOS IO Proc
     *  info because the image file IS a DOS file.
     */
    if( mmFormatInfo.fccIOProc == FOURCC_DOS )
    {
            return ( 0L );
    }
    /*
     *  Ensure this is an IMAGE IOproc, and that it can read
     *  translated data
     */
    if ( (mmFormatInfo.ulMediaType != MMIO_MEDIATYPE_IMAGE) ||
         ((mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) == 0) )
    {
            return (0L);
    }
    else
    {
         fccIOProc = mmFormatInfo.fccIOProc;
    }

    /* Clear out and initialize mminfo structure */
    memset ( &mmioinfo, 0L, sizeof ( MMIOINFO ) );
    mmioinfo.fccIOProc = fccIOProc;
    mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
    hmmio = mmioOpen ( (PSZ) pszFileName,
                       &mmioinfo,
                       MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY );
    if ( ! hmmio )
    {
            return (0L);
    }


    dwReturnCode = mmioQueryHeaderLength ( hmmio,
                                         (PLONG)&ulImageHeaderLength,
                                           0L,
                                           0L);
    if ( ulImageHeaderLength != sizeof ( MMIMAGEHEADER ) )
    {
      /* We have a problem.....possibly incompatible versions */
      ulReturnCode = mmioClose (hmmio, 0L);
            return (0L);
    }

    ulReturnCode = mmioGetHeader ( hmmio,
                                   &mmImgHdr,
                                   (LONG) sizeof ( MMIMAGEHEADER ),
                                   (PLONG)&ulBytesRead,
                                   0L,
                                   0L);

    if ( ulReturnCode != MMIO_SUCCESS )
    {
      /* Header unavailable */
      ulReturnCode = mmioClose (hmmio, 0L);
            return (0L);
    }
    /*
    memcpy(pBMPInfoHeader2, &mmImgHdr.mmXDIBHeader.BMPInfoHeader2,
           sizeof(BITMAPINFOHEADER2)+256*sizeof(RGB2) );
           */
    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    /*
     *  Determine the number of bytes required, per row.
     *      PLANES MUST ALWAYS BE = 1
     */
    dwHeight = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy;
    dwWidth = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx;
    wBitCount = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwRowBits = dwWidth * mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwNumRowBytes = dwRowBits >> 3;

    /*
     *  Account for odd bits used in 1bpp or 4bpp images that are
     *  NOT on byte boundaries.
     */
    if ( dwRowBits % 8 )
    {
         dwNumRowBytes++;
    }
    /*
     *  Ensure the row length in bytes accounts for byte padding.
     *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
     *  The data FROM an IOProc should always appear in this form.
     */
    dwPadBytes = ( dwNumRowBytes % 4 );
    if ( dwPadBytes )
    {
         dwNumRowBytes += 4 - dwPadBytes;
    }

    /* Allocate space for ONE row of pels */
    if ( DosAllocMem( (PPVOID)&pRowBuffer,
                      (ULONG)dwNumRowBytes,
                      fALLOC))
    {
      ulReturnCode = mmioClose (hmmio, 0L);
            return(0L);
    }

    /* Create a device context */
    hdc=DevOpenDC(hab, OD_MEMORY,"*",0L, NULL, NULLHANDLE);
    if(hdc==NULLHANDLE)
      {
        DosFreeMem(pRowBuffer);
        mmioClose (hmmio, 0L);
        return(0L);
      }
    
    /*
    // ***************************************************
    // Create a memory presentation space that includes
    // the memory device context obtained above.
    // ***************************************************
    */
    ImageSize.cx = dwWidth;
    ImageSize.cy = dwHeight;

    hps = GpiCreatePS ( hab,
                        hdc,
                        &ImageSize,
                        PU_PELS | GPIT_NORMAL | GPIA_ASSOC );
       if ( !hps )
      {
#ifdef DEBUG
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                       "No HPS...",
                       "Open Image File",
                       (HMODULE) NULL,
                       (ULONG) MB_OK | MB_MOVEABLE |
                       MB_ERROR );
#endif   
        DevCloseDC(hdc);
        DosFreeMem(pRowBuffer);
        mmioClose (hmmio, 0L);
        return(0L);
      }
       /*
    //    GpiSelectPalette(hps, NULLHANDLE);
    // ***************************************************
    // Create an uninitialized bitmap.  This is where we
    // will put all of the bits once we read them in.
    // ***************************************************
    */

    hbm = GpiCreateBitmap ( hps,
                            &mmImgHdr.mmXDIBHeader.BMPInfoHeader2,
                            0L,
                            NULL,
                            NULL);

#if 0
    hbm = GpiCreateBitmap ( hps,
                            pBMPInfoHeader2,
                            0L,
                            NULL,
                            NULL);
#endif

    if ( !hbm )
    {
#ifdef DEBUG
      WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                     "No HBITMAP...",
                     "Open Image File",
                     (HMODULE) NULL,
                     (ULONG) MB_OK | MB_MOVEABLE |
                     MB_ERROR );
#endif
      GpiDestroyPS(hps);
      DevCloseDC(hdc);
      DosFreeMem(pRowBuffer);
      ulReturnCode = mmioClose (hmmio, 0L);
      return(0L);
    }
    /*
    // ***************************************************
    // Select the bitmap into the memory device context.
    // ***************************************************/
    hbReturnCode = GpiSetBitmap ( hps,
                                  hbm );
    /*
    //***************************************************************
    //  LOAD THE BITMAP DATA FROM THE FILE
    //      One line at a time, starting from the BOTTOM
    //*************************************************************** */

    for ( dwRowCount = 0; dwRowCount < dwHeight; dwRowCount++ )
    {
         ulBytesRead = (ULONG) mmioRead ( hmmio,
                                          pRowBuffer,
                                          dwNumRowBytes );
         if ( !ulBytesRead )
         {
              break;
         }
         /*
          *  Allow context switching while previewing.. Couldn't get
          *  it to work. Perhaps will get to it when time is available...
          */
         lReturnCode = GpiSetBitmapBits ( hps,
                                          (LONG) dwRowCount,
                                          (LONG) 1,
                                          (PBYTE) pRowBuffer,
                                          (PBITMAPINFO2) &mmImgHdr.mmXDIBHeader.BMPInfoHeader2);
    }

    /* Clean up */
    hbReturnCode = GpiSetBitmap ( hps,
                                  NULLHANDLE );
    ulReturnCode = mmioClose (hmmio, 0L);
    DosFreeMem(pRowBuffer);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    return(hbm);
}

BOOL MyTrackRoutine(HWND hwnd, PRECTL rcl, ULONG ulFlag)
 {
   TRACKINFO track;
   HAB hab;
   HPS hps;

   track.cxBorder = 2;
   track.cyBorder = 2;  /* 4 pel wide lines used for rectangle */
   track.cxGrid = 1;
   track.cyGrid = 1;    /* smooth tracking with mouse */
   track.cxKeyboard = 8;
   track.cyKeyboard = 8; /* faster tracking using cursor keys */
 
   hab=WinQueryAnchorBlock(hwnd);

   hps=WinGetPS(hwnd);

   WinCopyRect(hab, &track.rclTrack, rcl);   /* starting point */

   WinQueryWindowRect(hwnd, &track.rclBoundary);
 
   track.ptlMinTrackSize.x = 10;
   track.ptlMinTrackSize.y = 10;  /* set smallest allowed size of rectangle */
 
   track.ptlMaxTrackSize.x = track.rclBoundary.xRight;
   track.ptlMaxTrackSize.y = track.rclBoundary.yTop; /* set largest allowed size of rectangle */
 
   track.fs = ulFlag;

   track.fs|=TF_ALLINBOUNDARY;
   if (WinTrackRect(hwnd, hps, &track) )
   {
     /* if successful copy final position back */
     WinCopyRect(hab, rcl, &track.rclTrack);
     WinReleasePS(hps);
     return(TRUE);
   }
   else
   {
     WinReleasePS(hps);
     return(FALSE);
   }
 }

/* There can only one tracking window at a time so this may be global */
BOOL bTrack=FALSE;

/************************************************************/
/*                                                          */
/* This function checks if the mouse pointer is over the    */
/* border of a selection or inside and sets the pointer     */
/* accordingly.                                             */
/*                                                          */
/* hwnd: image window                                       */
/* ptsStart: lower left corner of selection                 */
/* ptsTemp:  upper right corner of selection                */
/* xPtr, yPtr: current pointer position                     */
/*                                                          */
/************************************************************/
ULONG chkPointerInSelection( HWND hwnd, POINTS ptsStart, POINTS ptsTemp, SHORT xPtr, SHORT yPtr)
{
  RECTL rcl;
  RECTL rclTemp;
  HAB hab;
  POINTL ptl;

  ptl.x=xPtr;
  ptl.y=yPtr;

  if(ptsTemp.x>ptsStart.x)
    {
      rcl.xLeft=ptsStart.x;
      rcl.xRight=ptsTemp.x;
    }
  else
    {
      rcl.xLeft=ptsTemp.x;
      rcl.xRight=ptsStart.x;
    }
  if(ptsTemp.y>ptsStart.y)
    {
      rcl.yBottom=ptsStart.y;
      rcl.yTop=ptsTemp.y;
    }
  else
    {
      rcl.yBottom=ptsTemp.y;
      rcl.yTop=ptsStart.y;
    }

  hab=WinQueryAnchorBlock(hwnd);

  rclTemp=rcl;
  WinInflateRect(hab, &rclTemp, 2, 2);
  if(!WinPtInRect(hab, &rclTemp, &ptl))
    return 0; /* Mouse out of selection */

  rclTemp=rcl;
  WinInflateRect(hab, &rclTemp, -2, -2);
  if(WinPtInRect(hab, &rclTemp, &ptl))
    return 1; /* Mouse somewhere in the center of the selection */

  /* We are over the border */
  if(yPtr>rclTemp.yBottom && yPtr<rclTemp.yTop)
    {
      /* One of the vertical borders */
      WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE));
      if(xPtr>rclTemp.xLeft)
        return 2;
      else
        return 3;
    }
  else if((xPtr>rclTemp.xLeft && xPtr<rclTemp.xRight))
    {
      /* One of the horizontal borders */
      WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZENS, FALSE));
      if(yPtr>rclTemp.yBottom)
        return 4;
      else
        return 5;
    }

  return 0;
}

/************************************************************/
/*                                                          */
/* This function draws the selection rectangle into the     */
/* image window.                                            */
/*                                                          */
/* hwnd: image window                                       */
/* ptsStart: lower left corner of selection                 */
/* ptsTemp:  upper right corner of selection                */
/*                                                          */
/************************************************************/
BOOL drawSelection(HWND hwnd, POINTS ptsStart, POINTS ptsTemp)
{
  RECTL rcl;
  HPS hps;

  hps=WinGetPS(hwnd);
  if(!hps)
    return FALSE;

  if(ptsTemp.x>ptsStart.x)
    {
      rcl.xLeft=ptsStart.x;
      rcl.xRight=ptsTemp.x;
    }
  else
    {
      rcl.xLeft=ptsTemp.x;
      rcl.xRight=ptsStart.x;
    }
  if(ptsTemp.y>ptsStart.y)
    {
      rcl.yBottom=ptsStart.y;
      rcl.yTop=ptsTemp.y;
    }
  else
    {
      rcl.yBottom=ptsTemp.y;
      rcl.yTop=ptsStart.y;
    }
  WinDrawBorder(hps, &rcl, 1, 1, 0,0, DB_PATINVERT);
  WinReleasePS(hps);

  return TRUE;
}

/************************************************************/
/*                                                          */
/* This function draws the selection rectangle into the     */
/* image window.                                            */
/*                                                          */
/* hps: HPS from WinBeginPaint()                            */
/* ptsStart: lower left corner of selection                 */
/* ptsTemp:  upper right corner of selection                */
/*                                                          */
/************************************************************/
BOOL drawSelection2(HPS hps, POINTS ptsStart, POINTS ptsTemp)
{
  RECTL rcl;

  if(ptsTemp.x>ptsStart.x)
    {
      rcl.xLeft=ptsStart.x;
      rcl.xRight=ptsTemp.x;
    }
  else
    {
      rcl.xLeft=ptsTemp.x;
      rcl.xRight=ptsStart.x;
    }
  if(ptsTemp.y>ptsStart.y)
    {
      rcl.yBottom=ptsStart.y;
      rcl.yTop=ptsTemp.y;
    }
  else
    {
      rcl.yBottom=ptsTemp.y;
      rcl.yTop=ptsStart.y;
    }
  WinDrawBorder(hps, &rcl, 1, 1, 0,0, DB_PATINVERT);

  return TRUE;
}

/************************************************************/
/*                                                          */
/* Create a RECTL from two POINTS                           */
/*                                                          */
/************************************************************/
BOOL rectlFrom2Points(RECTL *rcl, POINTS ptsStart, POINTS ptsTemp)
{

  if(ptsTemp.x>ptsStart.x)
    {
      rcl->xLeft=ptsStart.x;
      rcl->xRight=ptsTemp.x;
    }
  else
    {
      rcl->xLeft=ptsTemp.x;
      rcl->xRight=ptsStart.x;
    }
  if(ptsTemp.y>ptsStart.y)
    {
      rcl->yBottom=ptsStart.y;
      rcl->yTop=ptsTemp.y;
    }
  else
    {
      rcl->yBottom=ptsTemp.y;
      rcl->yTop=ptsStart.y;
    }

  return TRUE;
}

BOOL orderStartAndEndPoint(POINTS *ptsStart, POINTS *ptsTemp)
{
  RECTL rcl;

  if(ptsTemp->x>ptsStart->x)
    {
      rcl.xLeft=ptsStart->x;
      rcl.xRight=ptsTemp->x;
    }
  else
    {
      rcl.xLeft=ptsTemp->x;
      rcl.xRight=ptsStart->x;
    }
  if(ptsTemp->y>ptsStart->y)
    {
      rcl.yBottom=ptsStart->y;
      rcl.yTop=ptsTemp->y;
    }
  else
    {
      rcl.yBottom=ptsTemp->y;
      rcl.yTop=ptsStart->y;
    }

  ptsStart->x=rcl.xLeft;
  ptsStart->y=rcl.yBottom;
  ptsTemp->x=rcl.xRight;
  ptsTemp->y=rcl.yTop;

  return TRUE;
}


/************************************************************/
/*                                                          */
/* Get a POINTS from the window ULONG.                      */
/*                                                          */
/************************************************************/
void ptsFromWindowULong(HWND hwnd, POINTS * ptsStart, POINTS*ptsTemp)
{
  ULONG ulTemp;

  ulTemp=WinQueryWindowULong(hwnd, QWL_PTSSTART);
  memcpy(ptsStart, &ulTemp, sizeof(ULONG));

  ulTemp=WinQueryWindowULong(hwnd, QWL_PTSTEMP);
  memcpy(ptsTemp, &ulTemp, sizeof(ULONG));

}

/************************************************************/
/*                                                          */
/* Write a POINTS to the window ULONG.                      */
/*                                                          */
/************************************************************/
void windowULongFromPts(HWND hwnd, POINTS ptsStart, POINTS ptsTemp)
{
  ULONG ulTemp;

  ulTemp=MAKELONG(ptsStart.x, ptsStart.y);
  WinSetWindowULong(hwnd, QWL_PTSSTART, (ULONG)ulTemp);

  ulTemp=MAKELONG(ptsTemp.x, ptsTemp.y);
  WinSetWindowULong(hwnd, QWL_PTSTEMP, (ULONG)ulTemp);
}

/*
 * This is the window proc for the image control
 */
static MRESULT EXPENTRY _imageProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  MRESULT mrc;
  HPS hps;
  PWNDPARAMS pwp;
  int iPercent;
  RECTL rcl;

  switch(msg) {
  case WM_QUERYDLGCODE:
    return (MRESULT)DLGC_STATIC;

  case WM_QUERYWINDOWPARAMS:
    {
      pwp=(PWNDPARAMS)mp1;
      if(pwp->fsStatus&WPM_CCHTEXT || pwp->fsStatus&WPM_TEXT) {
        if(pwp->fsStatus & WPM_CCHTEXT) {
          char * ptrText;
          if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            {
              pwp->cchText=strlen(ptrText);
            }
        }
        if(pwp->fsStatus & WPM_TEXT) {
          char * ptrText;
          if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            {
              strcpy(pwp->pszText,ptrText);
              pwp->cchText=strlen(ptrText);
            }
        }
        return MRTRUE;
      }
      break;
    }

  case WM_SETWINDOWPARAMS:
    {
      pwp=(PWNDPARAMS)mp1;
      if(pwp->fsStatus==WPM_TEXT) {
        if(pwp->pszText[0]=='#')
          {
            /* Control messages */
            switch(pwp->pszText[1])
              {      
              case 'E':
              case 'e':
                {
                  /* Flag for enabling the selection feature */
                  if(atoi(&pwp->pszText[2])==1)
                    WinSetWindowULong(hwnd, QWL_FLAGS, WinQueryWindowULong(hwnd, QWL_FLAGS)| 0x00000001);
                  else
                    WinSetWindowULong(hwnd, QWL_FLAGS, WinQueryWindowULong(hwnd, QWL_FLAGS)&~0x00000001);
                  break;
                }
              case 's':
              case 'S':
                {
                  POINTS ptsStart, ptsTemp;
                  POINTS ptsStart2, ptsTemp2;
                  int x1, y1 , x2, y2;
                  if(sscanf(&pwp->pszText[2], "%d %d %d %d", &x1, &y1, &x2, &y2)==4)
                    {
                      /* Got new selection values. Now validate them. */
                      RECTL rcl;
                      char * ptrText;

                      if(!WinQueryWindowRect(hwnd, &rcl))
                        break;
                      if(x1>rcl.xRight || y1>rcl.yTop)
                        break;

                      if(x2<x1 || y2<y1)
                        break;

                      /* Make sure slection is inside window */
                      if(x1<0)
                        x1=0;
                      if(y1<0)
                        y1=0;
                      if(x2>rcl.xRight)
                        x2=rcl.xRight;
                      if(y2>rcl.yTop)
                        y2=rcl.yTop;

                      ptsFromWindowULong(hwnd, &ptsStart2, &ptsTemp2);
                      /* Clear old selection */
                      if(WinQueryWindowULong(hwnd, QWL_HAVESELECTION))
                        drawSelection(hwnd, ptsStart2, ptsTemp2);
                      /* Set points into window word  */
                      ptsStart.x=x1;
                      ptsStart.y=y1;
                      ptsTemp.x=x2;
                      ptsTemp.y=y2;
                      if(x1!=x2 && y1!=y2)
                        WinSetWindowULong(hwnd, QWL_HAVESELECTION, TRUE);
                      else
                        WinSetWindowULong(hwnd, QWL_HAVESELECTION, FALSE);
                      windowULongFromPts(hwnd, ptsStart, ptsTemp);

                      /* Save the selection data as a string */
                      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
                        sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y);
                      /* Draw newSelection */                    
                      drawSelection(hwnd, ptsStart, ptsTemp);
                    }
                  break;
                }
              default:
                break;
              }
            return (MRESULT)FALSE;
          }
        else
          {
            /* New image */
            HBITMAP hBitmap;
            ULONG ulStyle;
            char *ptrText;
            hBitmap=(HBITMAP)WinQueryWindowULong(hwnd, QWL_HBITMAP);
            if(hBitmap)
              GpiDeleteBitmap(hBitmap);
            
            hBitmap=loadBitmap(pwp->pszText);
            WinSetWindowULong(hwnd, QWL_HBITMAP, hBitmap);
            WinSetWindowULong(hwnd, QWL_STYLE, WinQueryWindowULong(hwnd, QWL_STYLE)|WS_PARENTCLIP);
            if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
              strcpy(ptrText,"0 0 0 0");
            WinSetWindowULong(hwnd, QWL_HAVESELECTION, FALSE);
            WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
            
            return (MRESULT)FALSE;
          }
      }
      break;
    }
  case WM_CREATE:
    {
      char * ptrText;
      POINTS ptsStart={0};
      POINTS ptsTemp={0};

      /* Set WS_PARENTCLIP or the tracking window will not be shown */
      WinSetWindowULong(hwnd, QWL_STYLE, WinQueryWindowULong(hwnd, QWL_STYLE)|WS_PARENTCLIP);

      windowULongFromPts(hwnd, ptsStart, ptsTemp);

      if((ptrText=malloc(CCHMAXPATH))!=NULLHANDLE)
        {
          strcpy(ptrText,"0 0 0 0");
          WinSetWindowULong(hwnd, QWL_TEXTPTR,(ULONG) ptrText);
        };
    break;
    }
  case WM_DESTROY:
    {
      HBITMAP hBitmap;
      char *ptrText;

      hBitmap=(HBITMAP)WinQueryWindowULong(hwnd, QWL_HBITMAP);
      if(hBitmap)
        GpiDeleteBitmap(hBitmap);

      /* Free the memory allocated for the text */
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        free(ptrText);

      break;
    }
  case WM_PAINT:
    {
      HPS hps;
      RECTL rectl, rcl;
      ULONG ulWidth, ulHeight, ulWidthWindow, ulHeightWindow, ulTemp, ulFact;
      ULONG ulPts;
      HBITMAP hBitmap;
      POINTS ptsStart, ptsTemp;

      hBitmap=(HBITMAP)WinQueryWindowULong(hwnd, QWL_HBITMAP);
      
      WinQueryUpdateRect(hwnd, &rcl);

      hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);

      WinFillRect(hps, &rcl, CLR_WHITE);
      
      WinQueryWindowRect(hwnd, &rectl);
      
      /*
        ulWidthWindow=rectl.xRight;
        ulHeightWindow=rectl.yTop;
        
        ulFact=ulWidth/ulWidthWindow>ulHeight/ulHeightWindow ? ulWidth*1000/ulWidthWindow : ulHeight*1000/ulHeightWindow;
        ulWidth=ulWidth*1000/ulFact;
        ulHeight=ulHeight*1000/ulFact;
        rectl.xRight=ulWidth;
        rectl.yTop=ulHeight;*/

      ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
      /*      WinDrawBitmap(hps, hBitmap, NULLHANDLE, (PPOINTL) &rectl, 0, 0, DBM_STRETCH);*/
      WinDrawBitmap(hps, hBitmap, NULLHANDLE, (PPOINTL) &rectl, CLR_WHITE, CLR_BLACK, DBM_STRETCH |DBM_IMAGEATTRS);
      if(WinQueryWindowULong(hwnd, QWL_HAVESELECTION))
        drawSelection2(hps, ptsStart, ptsTemp); 
      
      WinEndPaint(hps);
    return (MRESULT) FALSE;
    }
  case WM_MOUSEMOVE:
    {
      HPS hps;
      RECTL rcl;
      SHORT x, y;
      BOOL bHaveSelection=WinQueryWindowULong(hwnd, QWL_HAVESELECTION);
      
      if(bTrack) {          
        hps=WinGetPS(hwnd);
        if(hps)
          {
            POINTS pts , ptsStart, ptsTemp;

            ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
            drawSelection(hwnd, ptsStart, ptsTemp);

            pts.x=SHORT1FROMMP(mp1);
            pts.y=SHORT2FROMMP(mp1);
            drawSelection(hwnd, ptsStart, pts);

            ptsTemp.x=pts.x;
            ptsTemp.y=pts.y;

            windowULongFromPts(hwnd, ptsStart, ptsTemp);

            WinSetWindowULong(hwnd, QWL_HAVESELECTION, TRUE);
          }
        WinReleasePS(hps);
      }
      else if(bHaveSelection) {
        /* Check if mouse is over selection */
        RECTL rcl;
        POINTS ptsStart, ptsTemp;

        ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);

        switch(chkPointerInSelection( hwnd, ptsStart, ptsTemp, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1)))
          {
          case 2:
          case 3:
          case 4:
          case 5:
            return MRFALSE;/* We have set the mouse pointer */
          default:
            break;
          }
      }
        break;
      }
  case WM_BUTTON1CLICK:
    {
      HPS hps;
      char *ptrText;
      POINTS ptsStart, ptsTemp;

      ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
      /* Clear selection */
      drawSelection(hwnd, ptsStart, ptsTemp); 
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        strcpy(ptrText,"0 0 0 0");
      ptsStart.x=ptsStart.y=0;
      ptsTemp.x=ptsTemp.y=0;
      WinSetWindowULong(hwnd, QWL_HAVESELECTION, FALSE);
      windowULongFromPts(hwnd, ptsStart, ptsTemp);
      bTrack=FALSE;
      break;
    }
  case WM_BUTTON1UP:
    {
      char *ptrText;
      POINTS ptsStart, ptsTemp;

      /* Selection done */
      bTrack=FALSE;
      
      ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);

      /* Make sure start and end point are in the order PM uses rectangles.
         This means, ptsStart is lower left point and ptsTemp is upper right. */
      orderStartAndEndPoint(&ptsStart, &ptsTemp);

      /* Save the selection data as a string */
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y);
      
      break;
    }
    case WM_BUTTON2MOTIONSTART:
      {
        BOOL bHaveSelection=WinQueryWindowULong(hwnd, QWL_HAVESELECTION);

        if(bHaveSelection) {
          RECTL rcl;
          char *ptrText;
          POINTS ptsStart, ptsTemp;

          ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
          switch(chkPointerInSelection( hwnd, ptsStart, ptsTemp, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1)))
            {
            case 1:
              WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_MOVE, FALSE));
              rectlFrom2Points(&rcl, ptsStart, ptsTemp);
              MyTrackRoutine(hwnd, &rcl, TF_MOVE);
              ptsStart.x=rcl.xLeft;
              ptsStart.y=rcl.yBottom;
              ptsTemp.x=rcl.xRight;
              ptsTemp.y=rcl.yTop;
              windowULongFromPts(hwnd, ptsStart, ptsTemp);
              if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
                {
                  sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y);
                };
              break;
            case 2:
              break;
            default:
              break;
            }
        }
        break;
      }
    case WM_BUTTON1MOTIONSTART:
      {
        POINTS ptsStart, ptsTemp;
        BOOL bHaveSelection=WinQueryWindowULong(hwnd, QWL_HAVESELECTION);

        if(!(WinQueryWindowULong(hwnd, QWL_FLAGS)& 0x00000001))
          return MRFALSE;


        if(bHaveSelection) {
          RECTL rcl;
          char *ptrText;

          ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
          rectlFrom2Points(&rcl, ptsStart, ptsTemp);
          switch(chkPointerInSelection( hwnd, ptsStart, ptsTemp, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1)))
            {
            case 2:
              MyTrackRoutine(hwnd, &rcl, TF_RIGHT);
              break;
            case 3:
              MyTrackRoutine(hwnd, &rcl, TF_LEFT);
              break;
            case 4:
              MyTrackRoutine(hwnd, &rcl, TF_TOP);
              break;
            case 5:
              MyTrackRoutine(hwnd, &rcl, TF_BOTTOM);
              break;
            default:
              /* Clear previous selection*/
              drawSelection(hwnd, ptsStart, ptsTemp);
              ptsStart.x=SHORT1FROMMP(mp1);
              ptsStart.y=SHORT2FROMMP(mp1);
              
              ptsTemp=ptsStart;
              windowULongFromPts(hwnd, ptsStart, ptsTemp);
              bTrack=TRUE;
              return MRFALSE;
            }
          ptsStart.x=rcl.xLeft;
          ptsStart.y=rcl.yBottom;
          ptsTemp.x=rcl.xRight;
          ptsTemp.y=rcl.yTop;

          windowULongFromPts(hwnd, ptsStart, ptsTemp);
          if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
              sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y);

          return MRFALSE;
        }
        ptsStart.x=SHORT1FROMMP(mp1);
        ptsStart.y=SHORT2FROMMP(mp1);
        
        ptsTemp=ptsStart;

        windowULongFromPts(hwnd, ptsStart, ptsTemp);

        ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
        bTrack=TRUE;
        return MRFALSE;
      }  
  default:
    break;
  }

  mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
  return (mrc);
}

typedef struct _hist
{
  ULONG ulMax;
  ULONG grey[256];
}HISTOGRAM;

#define HISTOGRAM_HEIGHT 200
#define HISTOGRAM_WIDTH 256
/*************************************************************************/
/*                                                                       */
/*   Functions for the histogram control                                 */
/*                                                                       */
/*************************************************************************/
HBITMAP getHistogramBMP(  PSZ pszFileName, ULONG ulFG, ULONG ulBG /*,PBITMAPINFOHEADER2 pBMPInfoHeader2*/)
{
  HBITMAP       hbm;  
  MMIOINFO      mmioinfo;
  MMFORMATINFO  mmFormatInfo;
  HMMIO         hmmio;
  ULONG         ulImageHeaderLength;
  MMIMAGEHEADER mmImgHdr;
  ULONG         ulBytesRead;
  PBYTE         pRowBuffer;
  PBYTE         pRowBuffer24Bit;
  ULONG         dwRowCount;
  SIZEL         ImageSize;
  ULONG         dwHeight, dwWidth;
  SHORT          wBitCount;
  FOURCC        fccStorageSystem;
  ULONG         dwPadBytes;
  ULONG         dwRowBits;
  ULONG         dwNumRowBytes;
  ULONG         ulReturnCode;
  HBITMAP       hbReturnCode;
  LONG          lReturnCode;
  FOURCC        fccIOProc;
  HDC           hdc;
  HPS           hps;
  HAB           hab;
  
    RECTL         rectl={0};
    HISTOGRAM    hist={0};
    BITMAPINFOHEADER2 bmpih2={0};

    hab=WinQueryAnchorBlock(HWND_DESKTOP);

    ulReturnCode = mmioIdentifyFile ( pszFileName,
                                      0L,
                                      &mmFormatInfo,
                                      &fccStorageSystem,
                                      0L,
                                      0L);
    /*
     *  If this file was NOT identified, then this function won't
     *  work, so return an error by indicating an empty bitmap.
     */
    if ( ulReturnCode == MMIO_ERROR )
    {
            return (0L);
    }
    /*
     *  If mmioIdentifyFile did not find a custom-written IO proc which
     *  can understand the image file, then it will return the DOS IO Proc
     *  info because the image file IS a DOS file.
     */
    if( mmFormatInfo.fccIOProc == FOURCC_DOS )
    {
            return ( 0L );
    }
    /*
     *  Ensure this is an IMAGE IOproc, and that it can read
     *  translated data
     */
    if ( (mmFormatInfo.ulMediaType != MMIO_MEDIATYPE_IMAGE) ||
         ((mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) == 0) )
    {
            return (0L);
    }
    else
    {
         fccIOProc = mmFormatInfo.fccIOProc;
    }

    /* Clear out and initialize mminfo structure */
    memset ( &mmioinfo, 0L, sizeof ( MMIOINFO ) );
    mmioinfo.fccIOProc = fccIOProc;
    mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
    hmmio = mmioOpen ( (PSZ) pszFileName,
                       &mmioinfo,
                       MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY );
    if ( ! hmmio )
    {
            return (0L);
    }


    ulReturnCode = mmioQueryHeaderLength ( hmmio,
                                         (PLONG)&ulImageHeaderLength,
                                           0L,
                                           0L);
    if ( ulImageHeaderLength != sizeof ( MMIMAGEHEADER ) )
    {
      /* We have a problem.....possibly incompatible versions */
      ulReturnCode = mmioClose (hmmio, 0L);
            return (0L);
    }

    ulReturnCode = mmioGetHeader ( hmmio,
                                   &mmImgHdr,
                                   (LONG) sizeof ( MMIMAGEHEADER ),
                                   (PLONG)&ulBytesRead,
                                   0L,
                                   0L);

    if ( ulReturnCode != MMIO_SUCCESS )
    {
      /* Header unavailable */
      ulReturnCode = mmioClose (hmmio, 0L);
            return (0L);
    }

    /*
     *  Determine the number of bytes required, per row.
     *      PLANES MUST ALWAYS BE = 1
     */
    dwHeight = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy;
    dwWidth = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx;
    wBitCount = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwRowBits = dwWidth * mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwNumRowBytes = dwRowBits >> 3;

    /*
     *  Account for odd bits used in 1bpp or 4bpp images that are
     *  NOT on byte boundaries.
     */
    if ( dwRowBits % 8 )
    {
         dwNumRowBytes++;
    }
    /*
     *  Ensure the row length in bytes accounts for byte padding.
     *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
     *  The data FROM an IOProc should always appear in this form.
     */
    dwPadBytes = ( dwNumRowBytes % 4 );
    if ( dwPadBytes )
    {
         dwNumRowBytes += 4 - dwPadBytes;
    }

    /* Allocate space for ONE row of pels */
    if ( DosAllocMem( (PPVOID)&pRowBuffer,
                      (ULONG)dwNumRowBytes,
                      fALLOC))
    {
      ulReturnCode = mmioClose (hmmio, 0L);
      return(0L);
    }

#if 0
    /* 24 bit memory for image data */
    dwRowBits = dwWidth *24 ;
    dwNumRowBytes = dwRowBits >> 3;
    dwPadBytes = ( dwNumRowBytes % 4 );

    /*
     *  Ensure the row length in bytes accounts for byte padding.
     *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
     *  The data FROM an IOProc should always appear in this form.
     */
    if ( dwPadBytes )
    {
         dwNumRowBytes += 4 - dwPadBytes;
    }
    /* Allocate space for ONE row of pels */
    if ( DosAllocMem( (PPVOID)&pRowBuffer24Bit,
                      (ULONG)dwNumRowBytes,
                      fALLOC))
    {
      ulReturnCode = mmioClose (hmmio, 0L);
      DosFreeMem(pRowBuffer);
      return(0L);
    }
#endif

    /* Create a device context */
    hdc=DevOpenDC(hab, OD_MEMORY,"*",0L, NULL, NULLHANDLE);
    if(hdc==NULLHANDLE)
      {
        DosFreeMem(pRowBuffer);
        mmioClose (hmmio, 0L);
        return(0L);
      }
    
    /*
    // ***************************************************
    // Create a memory presentation space that includes
    // the memory device context obtained above.
    // ***************************************************
    */
    /*    ImageSize.cx = dwWidth;
          ImageSize.cy = dwHeight;*/

    ImageSize.cx = HISTOGRAM_WIDTH;
    ImageSize.cy = HISTOGRAM_HEIGHT;

    hps = GpiCreatePS ( hab,
                        hdc,
                        &ImageSize,
                        PU_PELS | GPIT_NORMAL | GPIA_ASSOC );
    if ( !hps )
      {
#ifdef DEBUG
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                       "No HPS...",
                       "Open Image File",
                       (HMODULE) NULL,
                       (ULONG) MB_OK | MB_MOVEABLE |
                       MB_ERROR );
#endif   
        DevCloseDC(hdc);
        DosFreeMem(pRowBuffer);
        mmioClose (hmmio, 0L);
        return(0L);
      }
    /*
      // ***************************************************
      // Create an uninitialized bitmap.  This is where we
      // will put all of the bits once we read them in.
      // ***************************************************
      */
    
    bmpih2.cbFix=sizeof(BITMAPINFOHEADER2);
    bmpih2.cx=ImageSize.cx;
    bmpih2.cy=ImageSize.cy;
    bmpih2.cPlanes=1;
    bmpih2.cBitCount=8;
    bmpih2.cxResolution=ImageSize.cx;
    bmpih2.cyResolution=ImageSize.cy;
    bmpih2.cclrUsed=1;

    hbm = GpiCreateBitmap ( hps,
                            /*    &mmImgHdr.mmXDIBHeader.BMPInfoHeader2,*/
                            &bmpih2,
                            0L,
                            NULL,
                            NULL);
    
    if ( !hbm )
      {
#ifdef DEBUG
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                       "No HBITMAP...",
                       "Open Image File",
                       (HMODULE) NULL,
                       (ULONG) MB_OK | MB_MOVEABLE |
                       MB_ERROR );
#endif
        GpiDestroyPS(hps);
        DevCloseDC(hdc);
        DosFreeMem(pRowBuffer);
        ulReturnCode = mmioClose (hmmio, 0L);
        return(0L);
      }
    /*
      // ***************************************************
      // Select the bitmap into the memory device context.
      // **************************************************
      */
    hbReturnCode = GpiSetBitmap ( hps,
                                  hbm );
    /*
      //***************************************************************
         //  LOAD THE BITMAP DATA FROM THE FILE
         //      One line at a time, starting from the BOTTOM
         //*************************************************************** 
           */
    
    for ( dwRowCount = 0; dwRowCount < dwHeight; dwRowCount++ )
      {
        
        ulBytesRead = (ULONG) mmioRead ( hmmio,
                                         pRowBuffer,
                                         dwNumRowBytes );
        if ( !ulBytesRead )
          {
            break;
          }
        /*
         *  Allow context switching while previewing.. Couldn't get
         *  it to work. Perhaps will get to it when time is available...
         */
        /*
          lReturnCode = GpiSetBitmapBits ( hps,
          (LONG) dwRowCount,
          (LONG) 1,
          (PBYTE) pRowBuffer,
          (PBITMAPINFO2) &mmImgHdr.mmXDIBHeader.BMPInfoHeader2);
          */
        
        if(mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount==24)
          {
            int a;
            BYTE * src;
            ULONG ulValue=0;
            
            src=pRowBuffer;
            for(a=0;a<ulBytesRead/3; a++)
              {
                ulValue=0;

                ulValue+=*src++;
                ulValue+=*src++;
                ulValue+=*src++;
                ulValue/=3;
                ulValue&=0xFF;
                hist.grey[ulValue]+=1;
                if(hist.grey[ulValue]>hist.ulMax)
                  hist.ulMax=hist.grey[ulValue];
              }
          }
      }
    
    rectl.yTop=HISTOGRAM_HEIGHT;
    rectl.xRight=HISTOGRAM_WIDTH;

    GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULLHANDLE);
    GpiSetColor(hps, ulFG);
    GpiSetBackColor(hps, ulBG);

    /* Set background */
    WinFillRect(hps, &rectl, ulBG);

    if(mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount==24)
      {
        int a;
        float f;

        f=hist.ulMax/HISTOGRAM_HEIGHT;

        for(a=0; a<256;a++)
          {
            POINTL ptl;

            ptl.x=a;
            ptl.y=0;
            GpiMove(hps, &ptl);

            ptl.y=((float)hist.grey[a])/f;
            /*    GpiSetColor(hps, CLR_RED);
                  if(a==255) {
                  GpiSetColor(hps, CLR_GREEN);
                  ptl.y=240;
                  }
                  if(a==0) {
                  GpiSetColor(hps, CLR_BLUE);
                  ptl.y=240;
                  }
                  */
            GpiLine(hps, &ptl);
          }
      }

    /* Clean up */
    hbReturnCode = GpiSetBitmap ( hps,
                                  NULLHANDLE );
    ulReturnCode = mmioClose (hmmio, 0L);
    DosFreeMem(pRowBuffer);
    GpiDestroyPS(hps);
    DevCloseDC(hdc);

    return(hbm);
}

BOOL histogramTrackRoutine(HWND hwnd, PRECTL rcl, ULONG ulFlag)
 {
   TRACKINFO track;
   HAB hab;
   HPS hps;

   track.cxBorder = 1;
   track.cyBorder = 0;  /* 4 pel wide lines used for rectangle */
   track.cxGrid = 1;
   track.cyGrid = 1;    /* smooth tracking with mouse */
   track.cxKeyboard = 8;
   track.cyKeyboard = 8; /* faster tracking using cursor keys */
 
   hab=WinQueryAnchorBlock(hwnd);

   hps=WinGetPS(hwnd);

   WinQueryWindowRect(hwnd, &track.rclBoundary);

   rcl->yBottom=0;
   rcl->yTop=track.rclBoundary.yTop;
   WinCopyRect(hab, &track.rclTrack, rcl);   /* starting point */
 
   track.ptlMinTrackSize.x = 1;
   track.ptlMinTrackSize.y = 50;  /* set smallest allowed size of rectangle */
 
   track.ptlMaxTrackSize.x = track.rclBoundary.xRight-1;
   track.ptlMaxTrackSize.y = track.rclBoundary.yTop; /* set largest allowed size of rectangle */
 
   track.fs = ulFlag;

   track.fs|=TF_ALLINBOUNDARY;
   if (WinTrackRect(hwnd, hps, &track) )
   {
     /* if successful copy final position back */
     WinCopyRect(hab, rcl, &track.rclTrack);
     WinReleasePS(hps);
     return(TRUE);
   }
   else
   {
     WinReleasePS(hps);
     return(FALSE);
   }
 }

#define HIST_FLAG_VERTICAL   0x00000001
#define HIST_FLAG_HORIZONTAL 0x00000002
/************************************************************/
/*                                                          */
/* This function checks if the mouse pointer is over the    */
/* border of a selection or inside and sets the pointer     */
/* accordingly.                                             */
/*                                                          */
/* hwnd: image window                                       */
/* ptsStart: lower left corner of selection                 */
/* ptsTemp:  upper right corner of selection                */
/* xPtr, yPtr: current pointer position                     */
/*                                                          */
/************************************************************/
ULONG chkPointerInSelectionHist( HWND hwnd, POINTS ptsStart, POINTS ptsTemp, SHORT xPtr, SHORT yPtr, ULONG ulFlag)
{
  RECTL rcl;
  RECTL rclTemp;
  HAB hab;
  POINTL ptl;

  ptl.x=xPtr;
  ptl.y=yPtr;

  WinQueryWindowRect(hwnd, &rcl);

  if(ptsTemp.x>ptsStart.x)
    {
      rcl.xLeft=ptsStart.x;
      rcl.xRight=ptsTemp.x;
    }
  else
    {
      rcl.xLeft=ptsTemp.x;
      rcl.xRight=ptsStart.x;
    }

  hab=WinQueryAnchorBlock(hwnd);

  rclTemp=rcl;
  WinInflateRect(hab, &rclTemp, 2, 2);
  if(!WinPtInRect(hab, &rclTemp, &ptl))
    return 0; /* Mouse out of selection */

  rclTemp=rcl;
  WinInflateRect(hab, &rclTemp, -2, -2);
  if(WinPtInRect(hab, &rclTemp, &ptl))
    return 1; /* Mouse somewhere in the center of the selection */

  /* We are over the border */
  if(yPtr>rclTemp.yBottom && yPtr<rclTemp.yTop)
    {
      /* One of the vertical borders */
      if( ulFlag & HIST_FLAG_VERTICAL)
        WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZEWE, FALSE));
      if(xPtr>rclTemp.xLeft)
        return 2;
      else
        return 3;
    }
  else if((xPtr>rclTemp.xLeft && xPtr<rclTemp.xRight))
    {
      /* One of the horizontal borders */
      if( ulFlag & HIST_FLAG_HORIZONTAL)
        WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_SIZENS, FALSE));
      if(yPtr>rclTemp.yBottom)
        return 4;
      else
        return 5;
    }

  return 0;
}

/************************************************************/
/*                                                          */
/* This function draws the selection rectangle into the     */
/* image window.                                            */
/*                                                          */
/* hwnd: image window                                       */
/* ptsStart: lower left corner of selection                 */
/* ptsTemp:  upper right corner of selection                */
/*                                                          */
/************************************************************/
BOOL drawHistSelection(HWND hwnd, POINTS ptsStart, POINTS ptsTemp)
{
  RECTL rcl;
  HPS hps;

  hps=WinGetPS(hwnd);
  if(!hps)
    return FALSE;

  WinQueryWindowRect(hwnd, &rcl);
  /*  HlpWriteToTrapLog("%s: temp: %d start: %d\n", __FUNCTION__, ptsTemp.x, ptsStart.x);*/
  if(ptsTemp.x>ptsStart.x)
    {
      rcl.xLeft=ptsStart.x;
      rcl.xRight=ptsTemp.x;
    }
  else
    {
      rcl.xLeft=ptsTemp.x;
      rcl.xRight=ptsStart.x;
    }
  rcl.xRight+=1;
  /*  HlpWriteToTrapLog("%s:  %d %d %d %d\n", __FUNCTION__, rcl.xLeft, rcl.xRight, rcl.yBottom, rcl.yTop);*/
  WinDrawBorder(hps, &rcl, 1, 0, 0,0, DB_PATINVERT);
  WinReleasePS(hps);

  return TRUE;
}

/************************************************************/
/*                                                          */
/* This function draws the selection rectangle into the     */
/* image window.                                            */
/*                                                          */
/* hps: HPS from WinBeginPaint()                            */
/* ptsStart: lower left corner of selection                 */
/* ptsTemp:  upper right corner of selection                */
/*                                                          */
/************************************************************/
BOOL drawHistSelection2(HPS hps, HWND hwnd, POINTS ptsStart, POINTS ptsTemp)
{
  RECTL rcl;

  WinQueryWindowRect(hwnd, &rcl);

  /*  HlpWriteToTrapLog("%s: temp: %d start: %d\n", __FUNCTION__, ptsTemp.x, ptsStart.x);*/

  if(ptsTemp.x>ptsStart.x)
    {
      rcl.xLeft=ptsStart.x;
      rcl.xRight=ptsTemp.x;
    }
  else
    {
      rcl.xLeft=ptsTemp.x;
      rcl.xRight=ptsStart.x;
    }

  rcl.xRight+=1;

  /*  HlpWriteToTrapLog("%s:  %d %d %d %d\n", __FUNCTION__, rcl.xLeft, rcl.xRight, rcl.yBottom, rcl.yTop);*/
  WinDrawBorder(hps, &rcl, 1, 0, 0,0, DB_PATINVERT);

  return TRUE;
}


/*
 * This is the window proc for the histogram control
 */
static MRESULT EXPENTRY _histogramProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  MRESULT mrc;
  HPS hps;
  PWNDPARAMS pwp;
  int iPercent;
  RECTL rcl;

  switch(msg) {
  case WM_QUERYDLGCODE:
    return (MRESULT)DLGC_STATIC;

  case WM_QUERYWINDOWPARAMS:
    {
      pwp=(PWNDPARAMS)mp1;
      if(pwp->fsStatus&WPM_CCHTEXT || pwp->fsStatus&WPM_TEXT) {
        if(pwp->fsStatus & WPM_CCHTEXT) {
          char * ptrText;
          if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            {
              pwp->cchText=strlen(ptrText);
            }
        }
        if(pwp->fsStatus & WPM_TEXT) {
          char * ptrText;
          if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            {
              strcpy(pwp->pszText,ptrText);
              pwp->cchText=strlen(ptrText);
            }
        }
        return MRTRUE;
      }
      break;
    }

  case WM_SETWINDOWPARAMS:
    {
      pwp=(PWNDPARAMS)mp1;
      if(pwp->fsStatus==WPM_TEXT) {
        if(pwp->pszText[0]=='#')
          {
            /* Control messages */
            switch(pwp->pszText[1])
              {      
              case 'E':
              case 'e':
                {
                  /* Flag for enabling the selection feature */
                  if(atoi(&pwp->pszText[2])==1)
                    WinSetWindowULong(hwnd, QWL_FLAGS, WinQueryWindowULong(hwnd, QWL_FLAGS)| 0x00000001);
                  else
                    WinSetWindowULong(hwnd, QWL_FLAGS, WinQueryWindowULong(hwnd, QWL_FLAGS)&~0x00000001);
                  break;
                }
              case 's':
              case 'S':
                {
                  POINTS ptsStart, ptsTemp;
                  POINTS ptsStart2, ptsTemp2;
                  int x1, y1 , x2, y2;
                  if(sscanf(&pwp->pszText[2], "%d %d", &x1, &x2)==2)
                    {
                      /* Got new selection values. Now validate them. */
                      RECTL rcl;
                      char * ptrText;

                      if(!WinQueryWindowRect(hwnd, &rcl))
                        break;

                      if(x1>rcl.xRight)
                        break;

                      if(x2<x1)
                        break;
                      /*HlpWriteToTrapLog("x2: %d %d\n", x2, rcl.xRight);*/
                      /* Make sure slection is inside window */
                      if(x1<0)
                        x1=0;
                      if(x2>rcl.xRight-1)
                        x2=rcl.xRight-1;

                      ptsFromWindowULong(hwnd, &ptsStart2, &ptsTemp2);
                      /* Clear old selection */
                      if(WinQueryWindowULong(hwnd, QWL_HAVESELECTION))
                        drawHistSelection(hwnd, ptsStart2, ptsTemp2);

                      /* Set points into window word  */
                      ptsStart.x=x1;
                      ptsStart.y=0;
                      ptsTemp.x=x2;
                      ptsTemp.y=rcl.yTop;
                      if(x1!=x2 && y1!=y2)
                        WinSetWindowULong(hwnd, QWL_HAVESELECTION, TRUE);
                      else
                        WinSetWindowULong(hwnd, QWL_HAVESELECTION, FALSE);
                      windowULongFromPts(hwnd, ptsStart, ptsTemp);

                      /* Save the selection data as a string */
                      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
                        /*   sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y); */
                        sprintf(ptrText,"%d %d", ptsStart.x, ptsTemp.x);
                      /* Draw newSelection */                    
                      drawHistSelection(hwnd, ptsStart, ptsTemp);
                    }
                  break;
                }
              default:
                break;
              }
            return (MRESULT)FALSE;
          }
        else
          {
            /* New image */
            HBITMAP hBitmap;
            ULONG ulStyle;
            char *ptrText;
            ULONG ulBG, ulFG;

            hBitmap=(HBITMAP)WinQueryWindowULong(hwnd, QWL_HBITMAP);
            if(hBitmap)
              GpiDeleteBitmap(hBitmap);
           
            /* Find color. This color is the background color set within DrDialog */
            if(!WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, sizeof(ulBG),
                                  &ulBG, QPF_ID2COLORINDEX|QPF_NOINHERIT ))
              ulBG=0x00ffffff;
            /* Find color. This color is the background color set within DrDialog */
            if(!WinQueryPresParam(hwnd, PP_FOREGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, sizeof(ulFG),
                                  &ulFG, QPF_ID2COLORINDEX|QPF_NOINHERIT ))
              ulFG=0x000000ff;

            hBitmap=getHistogramBMP(pwp->pszText, ulFG, ulBG);
            WinSetWindowULong(hwnd, QWL_HBITMAP, hBitmap);
            WinSetWindowULong(hwnd, QWL_STYLE, WinQueryWindowULong(hwnd, QWL_STYLE)|WS_PARENTCLIP);
            if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
              strcpy(ptrText,"0 0");
            WinSetWindowULong(hwnd, QWL_HAVESELECTION, FALSE);
            WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
            
            return (MRESULT)FALSE;
          }
      }
      break;
    }
  case WM_CREATE:
    {
      char * ptrText;
      POINTS ptsStart={0};
      POINTS ptsTemp={0};

      /* Set WS_PARENTCLIP or the tracking window will not be shown */
      WinSetWindowULong(hwnd, QWL_STYLE, WinQueryWindowULong(hwnd, QWL_STYLE)|WS_PARENTCLIP);

      windowULongFromPts(hwnd, ptsStart, ptsTemp);

      if((ptrText=malloc(CCHMAXPATH))!=NULLHANDLE)
        {
          strcpy(ptrText,"0 0");
          WinSetWindowULong(hwnd, QWL_TEXTPTR,(ULONG) ptrText);
        };
    break;
    }
  case WM_DESTROY:
    {
      HBITMAP hBitmap;
      char *ptrText;

      hBitmap=(HBITMAP)WinQueryWindowULong(hwnd, QWL_HBITMAP);
      if(hBitmap)
        GpiDeleteBitmap(hBitmap);

      /* Free the memory allocated for the text */
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        free(ptrText);

      break;
    }
  case WM_PAINT:
    {
      HPS hps;
      RECTL rectl, rcl;
      ULONG ulWidth, ulHeight, ulWidthWindow, ulHeightWindow, ulTemp, ulFact;
      ULONG ulPts;
      HBITMAP hBitmap;
      POINTS ptsStart, ptsTemp;

      /* Get bitmap handle from window words */
      hBitmap=(HBITMAP)WinQueryWindowULong(hwnd, QWL_HBITMAP);
      
      WinQueryUpdateRect(hwnd, &rcl);

      hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);

      WinFillRect(hps, &rcl, CLR_WHITE);
      
      /* Window size */
      WinQueryWindowRect(hwnd, &rectl);
      
      ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
      WinDrawBitmap(hps, hBitmap, NULLHANDLE, (PPOINTL) &rectl, CLR_WHITE, CLR_BLACK, DBM_STRETCH | DBM_IMAGEATTRS);
      if(WinQueryWindowULong(hwnd, QWL_HAVESELECTION))
        drawHistSelection2(hps, hwnd, ptsStart, ptsTemp); 
      
      WinEndPaint(hps);
    return (MRESULT) FALSE;
    }

  case WM_MOUSEMOVE:
    {
      HPS hps;
      RECTL rcl;
      SHORT x, y;
      BOOL bHaveSelection=WinQueryWindowULong(hwnd, QWL_HAVESELECTION);
      
      if(bTrack) {          
        hps=WinGetPS(hwnd);
        if(hps)
          {
            POINTS pts , ptsStart, ptsTemp;

            ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
            /* Clear old selection if any */
            if(ptsStart.x!=ptsTemp.x)
              drawHistSelection(hwnd, ptsStart, ptsTemp);

            pts.x=SHORT1FROMMP(mp1);
            pts.y=SHORT2FROMMP(mp1);
            drawHistSelection(hwnd, ptsStart, pts);

            ptsTemp.x=pts.x;
            ptsTemp.y=pts.y;

            windowULongFromPts(hwnd, ptsStart, ptsTemp);

            WinSetWindowULong(hwnd, QWL_HAVESELECTION, TRUE);
          }
        WinReleasePS(hps);
      }
      else if(bHaveSelection) {
        /* Check if mouse is over selection */
        RECTL rcl;
        POINTS ptsStart, ptsTemp;

        ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);

        switch(chkPointerInSelectionHist( hwnd, ptsStart, ptsTemp, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1),HIST_FLAG_VERTICAL))
          {
          case 2:
          case 3:
          case 4:
          case 5:
            return MRFALSE;/* We have set the mouse pointer */
          default:
            break;
          }
      }
        break;
      }
  case WM_BUTTON1CLICK:
    {
      HPS hps;
      char *ptrText;
      POINTS ptsStart, ptsTemp;

      ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);

      /* Clear selection */
      /*   drawHistSelection(hwnd, ptsStart, ptsTemp); */

      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        strcpy(ptrText,"0 0");
      ptsStart.x=ptsStart.y=0;
      ptsTemp.x=ptsTemp.y=0;
      WinSetWindowULong(hwnd, QWL_HAVESELECTION, FALSE);
      windowULongFromPts(hwnd, ptsStart, ptsTemp);
      bTrack=FALSE;
      break;
    }
  case WM_BUTTON1UP:
    {
      char *ptrText;
      POINTS ptsStart, ptsTemp;

      /* Selection done */
      bTrack=FALSE;

      ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);

      /* Make sure start and end point are in the order PM uses rectangles.
         This means, ptsStart is lower left point and ptsTemp is upper right. */
      orderStartAndEndPoint(&ptsStart, &ptsTemp);
      drawHistSelection(hwnd, ptsStart, ptsTemp); 
      /* Save the selection data as a string */
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        sprintf(ptrText,"%d %d", ptsStart.x, ptsTemp.x);
      /*         sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y); */
      
      break;
    }
    case WM_BUTTON2MOTIONSTART:
      {
        BOOL bHaveSelection=WinQueryWindowULong(hwnd, QWL_HAVESELECTION);

        if(bHaveSelection) {
          RECTL rcl;
          char *ptrText;
          POINTS ptsStart, ptsTemp;

          ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
          switch(chkPointerInSelection( hwnd, ptsStart, ptsTemp, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1)))
            {
            case 1:
              WinSetPointer(HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_MOVE, FALSE));
              rectlFrom2Points(&rcl, ptsStart, ptsTemp);
              histogramTrackRoutine(hwnd, &rcl, TF_MOVE);
              ptsStart.x=rcl.xLeft;
              ptsStart.y=rcl.yBottom;
              ptsTemp.x=rcl.xRight;
              ptsTemp.y=rcl.yTop;
              windowULongFromPts(hwnd, ptsStart, ptsTemp);
              if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
                sprintf(ptrText,"%d %d", ptsStart.x, ptsTemp.x);
              /*                sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y);*/
                
              break;
            case 2:
              break;
            default:
              break;
            }
        }
        break;
      }
    case WM_BUTTON1MOTIONEND:
      {
        POINTS ptsStart, ptsTemp;

        ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
        bTrack=FALSE;
              drawHistSelection(hwnd, ptsStart, ptsTemp);

        return MRFALSE;
      }  
    case WM_BUTTON1MOTIONSTART:
      {
        POINTS ptsStart, ptsTemp;
        BOOL bHaveSelection=WinQueryWindowULong(hwnd, QWL_HAVESELECTION);

        if(!(WinQueryWindowULong(hwnd, QWL_FLAGS)& 0x00000001))
          return MRFALSE;

        if(bHaveSelection) {
          RECTL rcl;
          char *ptrText;

          ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
          rectlFrom2Points(&rcl, ptsStart, ptsTemp);

          /* Clear previous selection*/
          drawHistSelection(hwnd, ptsStart, ptsTemp);

          switch(chkPointerInSelectionHist( hwnd, ptsStart, ptsTemp, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1), HIST_FLAG_VERTICAL))
            {
            case 2:
              histogramTrackRoutine(hwnd, &rcl, TF_RIGHT);
              break;
            case 3:
              histogramTrackRoutine(hwnd, &rcl, TF_LEFT);
              break;
            default:
              /* Clear previous selection*/
              /*    drawHistSelection(hwnd, ptsStart, ptsTemp); */
              ptsStart.x=SHORT1FROMMP(mp1);
              /*     ptsStart.y=SHORT2FROMMP(mp1); */
              ptsStart.y=0;
              ptsTemp=ptsStart;
              windowULongFromPts(hwnd, ptsStart, ptsTemp);
              bTrack=TRUE;
              return MRFALSE;
            }

          ptsStart.x=rcl.xLeft;
          ptsStart.y=rcl.yBottom;
          ptsTemp.x=rcl.xRight;
          ptsTemp.y=rcl.yTop;

          windowULongFromPts(hwnd, ptsStart, ptsTemp);
          if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
              sprintf(ptrText,"%d %d", ptsStart.x, ptsTemp.x);
          /*       sprintf(ptrText,"%d %d %d %d", ptsStart.x, ptsStart.y, ptsTemp.x, ptsTemp.y);*/

          return MRFALSE;
        }

        ptsStart.x=SHORT1FROMMP(mp1);
        ptsStart.y=SHORT2FROMMP(mp1);
        
        ptsTemp=ptsStart;

        windowULongFromPts(hwnd, ptsStart, ptsTemp);

        ptsFromWindowULong(hwnd, &ptsStart, &ptsTemp);
        bTrack=TRUE;
        return MRFALSE;
      }  

  default:
    break;
  }

  mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
  return (mrc);
}


/*************************************************************************/

#define MAX_VAR_LEN  270

typedef struct RxStemData {
    SHVBLOCK shvb;                     /* Request block for RxVar    */
    CHAR varname[MAX_VAR_LEN];         /* Buffer for the variable    */
                                       /* name                       */
    ULONG stemlen;                     /* Length of stem.            */
 } RXSTEMDATA;

LONG rxSetLongInStem(RXSTRING args, ULONG ulTail, char* chrTailString, LONG lValue)
{
  char text[20];
  RXSTEMDATA ldp={0};
  /*char name[200]={0};*/

  sprintf(text, "%ld", lValue);
                                       /* Initialize data area       */
  strcpy(ldp.varname, args.strptr);
  ldp.stemlen = args.strlength;
  strupr(ldp.varname);                 /* uppercase the name         */

  if (ldp.varname[ldp.stemlen-1] != '.')
    ldp.varname[ldp.stemlen++] = '.';

  /* add tailString if any, else only tail */
  if(chrTailString) {
    sprintf(ldp.varname+ldp.stemlen, "%d.%s", ulTail, chrTailString);
  }
  else   /* Add tail number to stem */
    sprintf(ldp.varname+ldp.stemlen, "%d", ulTail);

  /*strncpy(name,ldp.varname, );
   */

  ldp.shvb.shvnext = NULL;                           /* Only one request block */
  ldp.shvb.shvname.strptr = ldp.varname;             /* Var name               */
  ldp.shvb.shvname.strlength = strlen(ldp.varname);  /* RxString length        */
  ldp.shvb.shvnamelen = ldp.shvb.shvname.strlength;
  /* Set the value of the var */
  ldp.shvb.shvvalue.strptr = text;
  ldp.shvb.shvvalue.strlength = strlen(text);
  ldp.shvb.shvvaluelen = ldp.shvb.shvvalue.strlength;
  ldp.shvb.shvcode = RXSHV_SYSET;
  ldp.shvb.shvret = 0;
  if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN) {
    return INVALID_ROUTINE;      /* error on non-zero          */
  }

  return VALID_ROUTINE;
}


/*************************************************************************/
/*                                                                       */
/*   Functions to get the histogram data                                 */
/*                                                                       */
/*************************************************************************/
BOOL getHistogramData(  PSZ pszFileName, RXSTRING stem)
{
  HBITMAP       hbm;  
  MMIOINFO      mmioinfo;
  MMFORMATINFO  mmFormatInfo;
  HMMIO         hmmio;
  ULONG         ulImageHeaderLength;
  MMIMAGEHEADER mmImgHdr;
  ULONG         ulBytesRead;
  PBYTE         pRowBuffer;
  PBYTE         pRowBuffer24Bit;
  ULONG         dwRowCount;
  SIZEL         ImageSize;
  ULONG         dwHeight, dwWidth;
  SHORT          wBitCount;
  FOURCC        fccStorageSystem;
  ULONG         dwPadBytes;
  ULONG         dwRowBits;
  ULONG         dwNumRowBytes;
  ULONG         ulReturnCode;
  HBITMAP       hbReturnCode;
  LONG          lReturnCode;
  FOURCC        fccIOProc;
  HISTOGRAM    hist={0};

  int a;

    ulReturnCode = mmioIdentifyFile ( pszFileName,
                                      0L,
                                      &mmFormatInfo,
                                      &fccStorageSystem,
                                      0L,
                                      0L);
    /*
     *  If this file was NOT identified, then this function won't
     *  work, so return an error by indicating an empty bitmap.
     */
    if ( ulReturnCode == MMIO_ERROR )
    {
            return (0L);
    }
    /*
     *  If mmioIdentifyFile did not find a custom-written IO proc which
     *  can understand the image file, then it will return the DOS IO Proc
     *  info because the image file IS a DOS file.
     */
    if( mmFormatInfo.fccIOProc == FOURCC_DOS )
    {
            return ( 0L );
    }
    /*
     *  Ensure this is an IMAGE IOproc, and that it can read
     *  translated data
     */
    if ( (mmFormatInfo.ulMediaType != MMIO_MEDIATYPE_IMAGE) ||
         ((mmFormatInfo.ulFlags & MMIO_CANREADTRANSLATED) == 0) )
    {
            return (0L);
    }
    else
      fccIOProc = mmFormatInfo.fccIOProc;
    

    /* Clear out and initialize mminfo structure */
    memset ( &mmioinfo, 0L, sizeof ( MMIOINFO ) );
    mmioinfo.fccIOProc = fccIOProc;
    mmioinfo.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;
    hmmio = mmioOpen ( (PSZ) pszFileName,
                       &mmioinfo,
                       MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY );
    if ( ! hmmio )
      return (0L);
    

    ulReturnCode = mmioQueryHeaderLength ( hmmio,
                                           (PLONG)&ulImageHeaderLength,
                                           0L,
                                           0L);
    if ( ulImageHeaderLength != sizeof ( MMIMAGEHEADER ) )
    {
      /* We have a problem.....possibly incompatible versions */
      ulReturnCode = mmioClose (hmmio, 0L);
      return (0L);
    }

    ulReturnCode = mmioGetHeader ( hmmio,
                                   &mmImgHdr,
                                   (LONG) sizeof ( MMIMAGEHEADER ),
                                   (PLONG)&ulBytesRead,
                                   0L,
                                   0L);

    if ( ulReturnCode != MMIO_SUCCESS )
    {
      /* Header unavailable */
      ulReturnCode = mmioClose (hmmio, 0L);
      return (0L);
    }

    /*
     *  Determine the number of bytes required, per row.
     *      PLANES MUST ALWAYS BE = 1
     */
    dwHeight = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cy;
    dwWidth = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cx;
    wBitCount = mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwRowBits = dwWidth * mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount;
    dwNumRowBytes = dwRowBits >> 3;

    /*
     *  Account for odd bits used in 1bpp or 4bpp images that are
     *  NOT on byte boundaries.
     */
    if ( dwRowBits % 8 )
      {
        dwNumRowBytes++;
      }
    /*
     *  Ensure the row length in bytes accounts for byte padding.
     *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
     *  The data FROM an IOProc should always appear in this form.
     */
    dwPadBytes = ( dwNumRowBytes % 4 );
    if ( dwPadBytes )
      {
        dwNumRowBytes += 4 - dwPadBytes;
      }
    
    /* Allocate space for ONE row of pels */
    if ( DosAllocMem( (PPVOID)&pRowBuffer,
                      (ULONG)dwNumRowBytes,
                      fALLOC))
      {
        ulReturnCode = mmioClose (hmmio, 0L);
        return(0L);
      }
    
#if 0
    /* 24 bit memory for image data */
    dwRowBits = dwWidth *24 ;
    dwNumRowBytes = dwRowBits >> 3;
    dwPadBytes = ( dwNumRowBytes % 4 );

    /*
     *  Ensure the row length in bytes accounts for byte padding.
     *  All bitmap data rows must are aligned on LONG/4-BYTE boundaries.
     *  The data FROM an IOProc should always appear in this form.
     */
    if ( dwPadBytes )
    {
         dwNumRowBytes += 4 - dwPadBytes;
    }
    /* Allocate space for ONE row of pels */
    if ( DosAllocMem( (PPVOID)&pRowBuffer24Bit,
                      (ULONG)dwNumRowBytes,
                      fALLOC))
    {
      ulReturnCode = mmioClose (hmmio, 0L);
      DosFreeMem(pRowBuffer);
      return(0L);
    }
#endif
    
    /*
      //***************************************************************
         //  LOAD THE BITMAP DATA FROM THE FILE
         //      One line at a time, starting from the BOTTOM
         //*************************************************************** 
           */
    

    for ( dwRowCount = 0; dwRowCount < dwHeight; dwRowCount++ )
      {
        ulBytesRead = (ULONG) mmioRead ( hmmio,
                                         pRowBuffer,
                                         dwNumRowBytes );
        if ( !ulBytesRead )
          break;

        if(mmImgHdr.mmXDIBHeader.BMPInfoHeader2.cBitCount==24)
          {
            int a;
            BYTE * src;
            ULONG ulValue;
            
            src=pRowBuffer;

            for(a=0;a<ulBytesRead/3; a++)
              {
                ulValue=0;

                ulValue+=*src++;
                ulValue+=*src++;
                ulValue+=*src++;
                
                ulValue/=3;
                ulValue&=0x000000FF;
                
                hist.grey[ulValue]+=1;

                if(hist.grey[ulValue]>hist.ulMax)
                  hist.ulMax=hist.grey[ulValue];
              }

          }
        else {
          int a;

          for(a=0;a<256; a++)
            hist.grey[a]=0;
          hist.ulMax=0;
        }

      }

    for(a=0;a<256; a++)
      {
        if(rxSetLongInStem(stem, a+1, "_grey", hist.grey[a])==INVALID_ROUTINE)
          {
            rxSetLongInStem(stem, 0, NULLHANDLE, 0);
          }
      }

    rxSetLongInStem(stem, 0, NULLHANDLE, 256);

    ulReturnCode = mmioClose (hmmio, 0L);
    DosFreeMem(pRowBuffer);
    return(TRUE);
}


/*************************************************************************
* Function:  DRCtrlGetHistogram                                          *
*                                                                        *
*            Gets the histogram of an image                              *
*                                                                        *
*                                                                        *
* Syntax:    rc=DRCtrlGetHistogram(filename, stem)                       *
*                                                                        *
* Param1:    filename                                                    *
*                                                                        *
*            The image must be readable by OS/2.                         *
*                                                                        *
* Param2:    stem                                                        *
*                                                                        *
*            The stem containing the histogram.                          *
*                                                                        *
* Return:    0: failure, 1: success                                      *
*                                                                        *
* Remarks:                                                               *
*                                                                        *
*                                                                        *
*************************************************************************/
ULONG  DRCtrlGetHistogram(CHAR *name, ULONG numargs, RXSTRING args[],
                               CHAR *queuename, RXSTRING *retstr)
{

  retstr->strlength = 0;               /* set return value           */

  /* check arguments            */
  /* arg1:  image name */
  /* arg2:  STEM holding the result */
  if (numargs != 2)
    return INVALID_ROUTINE;

  sprintf(retstr->strptr, "%d", getHistogramData(  args[0].strptr, args[1]));
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}

/*************************************************************************/
/*                                                                       */
/*   Functions for the percent bar                                       */
/*                                                                       */
/*************************************************************************/
/*
 * Paint the percent bar and print the label if necessary.
 */
static VOID _paintPercent(int iPercent, HWND hwnd, HPS hps)
{
    POINTL  ptl, ptlText, aptlText[TXTBOX_COUNT];
    RECTL   rcl, rcl2;
    BOOL    bVertical=FALSE;
    CHAR  * ptrChr=NULL;

    WinQueryWindowRect(hwnd, &rcl);
    /* Check if it's a vertical percent bar */
    if(rcl.xRight<rcl.yTop)
      bVertical=TRUE;
    else
      bVertical=FALSE;

    GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
    
    /* Draw the bar border */
    WinDrawBorder(hps, &rcl, 1,1,0,0, 0x800);    
    
    rcl.xLeft = 1;
    rcl.xRight -= 1;
    rcl.yBottom = 1;
    rcl.yTop -= 1;
    
    if((ptrChr=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
      {
        /* Text size */
        GpiQueryTextBox(hps, strlen(ptrChr), ptrChr,
                        TXTBOX_COUNT, (PPOINTL)&aptlText);
   
        ptlText.x = rcl.xLeft+(((rcl.xRight-rcl.xLeft)
                                 -(aptlText[TXTBOX_BOTTOMRIGHT].x-aptlText[TXTBOX_BOTTOMLEFT].x))/2);
        ptlText.y = 3 + rcl.yBottom+(((rcl.yTop-rcl.yBottom)
                                      -(aptlText[TXTBOX_TOPLEFT].y-aptlText[TXTBOX_BOTTOMLEFT].y))/2);
      }

    if(!bVertical) {
      rcl2.xLeft = rcl.xLeft;
      rcl2.xRight = (rcl.xRight-rcl.xLeft)*iPercent/100; 
      rcl2.yBottom = rcl.yBottom;
      rcl2.yTop = rcl.yTop-1;
      rcl.xLeft=rcl2.xRight+1;
    }
    else {
      rcl2.xLeft = rcl.xLeft;
      rcl2.xRight = rcl.xRight-1;
      rcl2.yBottom = rcl.yBottom;
      rcl2.yTop = (rcl.yTop-rcl.yBottom)*iPercent/100; 
      rcl.yBottom=rcl2.yTop+1;
    }

    /* Background */
    WinFillRect(hps, &rcl,
                WinQuerySysColor(HWND_DESKTOP, SYSCLR_DIALOGBACKGROUND, 0));

    /* Percentbar */
    if ((rcl2.xRight > rcl2.xLeft && !bVertical)||(rcl2.yTop > rcl2.yBottom && bVertical)) {
      ULONG ulBG;

      /* Find color. This color is the background color set within DrDialog */
      if(!WinQueryPresParam(hwnd, PP_BACKGROUNDCOLOR, PP_BACKGROUNDCOLORINDEX, NULL, sizeof(ulBG),
                        &ulBG, QPF_ID2COLORINDEX|QPF_NOINHERIT ))
        ulBG=0x002020ff;
      GpiSetColor(hps,ulBG );

      rcl2.yBottom+=1;
      rcl2.xLeft+=1;

      WinFillRect(hps, &rcl2, ulBG);
      WinDrawBorder(hps, &rcl2, 1,1,0,0, 0x400);
    }

    /* now print the percentage */
    if(ptrChr!=NULLHANDLE)
      {
        ULONG ulFG; 
       
        /* Find color. This color is the foreground color set within DrDialog */
        if(!WinQueryPresParam(hwnd, PP_FOREGROUNDCOLOR, PP_FOREGROUNDCOLORINDEX, NULL, sizeof(ulFG),
                              &ulFG, QPF_ID2COLORINDEX|QPF_NOINHERIT ))
          ulFG=WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDEFAULT, 0);
        GpiSetColor(hps,ulFG );
        GpiMove(hps, &ptlText);
        GpiCharString(hps, strlen(ptrChr), ptrChr);
      }
}


/*
 * This is the window proc for the percentbar control
 */
static MRESULT EXPENTRY _percentBarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  MRESULT mrc;
  HPS hps;
  PWNDPARAMS pwp;
  int iPercent;
  RECTL rcl;

  switch(msg) {

  case WM_SETWINDOWPARAMS:
    {
      pwp=(PWNDPARAMS)mp1;
      if(pwp->fsStatus==WPM_TEXT) {
        /* The text changed */
        char *ptr;
        char *ptr2;

        /* Get the current percent value for the control */
        iPercent=atol(pwp->pszText);
        if(iPercent>100)
          iPercent=100;
        if(iPercent<0)
          iPercent=0;

        /* Check if there is some text for the bar */
        if((ptr=strchr(pwp->pszText, '#'))!=NULLHANDLE) {
          /* Everything after the '#' is treated as the label */
          if((ptr2=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            free(ptr2); /* Free the old text */
          WinSetWindowPtr(hwnd,QWL_TEXTPTR, NULLHANDLE);
          if(*(ptr++)!=0) {
            /* There's additional text to print */
            if((ptr2=malloc(strlen(ptr)+1))!=NULLHANDLE) {
              strcpy(ptr2,ptr);
              WinSetWindowPtr(hwnd,QWL_TEXTPTR,ptr2);
            }
          }
        }
        mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
        WinSetWindowULong(hwnd, QWL_PERCENT,iPercent);
        WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
        return mrc;
      }
      break;
    }
  case WM_DESTROY:
    {
      char *ptrText;
      /* Free the memory allocated for the text */
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        free(ptrText);
      break;
    }
  case WM_PRESPARAMCHANGED:
    /* The color or the font has changed  */
    /* Force a repaint of the percent bar */
    mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
    if(LONGFROMMP(mp1)==PP_FOREGROUNDCOLOR)
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
    else if(LONGFROMMP(mp1)==PP_BACKGROUNDCOLOR)
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
    else if(LONGFROMMP(mp1)==PP_FONTNAMESIZE)
      WinInvalidateRect(hwnd, NULLHANDLE,TRUE);
    return mrc;
  case WM_PAINT:
    {
      hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);
      _paintPercent(WinQueryWindowULong(hwnd,QWL_PERCENT), hwnd, hps);
      WinEndPaint(hps);
    return (MRESULT) FALSE;
    }
  
  default:
    break;
  }

  mrc = WinDefWindowProc(hwnd, msg, mp1, mp2);
  return (mrc);
}

#define shadowDeltaX 5
#define shadowDeltaY 5
MRESULT EXPENTRY _bubbleClientProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  switch(msg)
    {
    case  WM_PAINT:
      {
        HPS hps;
        RECTL rcl;
        POINTL aptlPoints[TXTBOX_COUNT];
        ULONG ulWinTextLen, ulLen;
        LONG   deltaX=0;
        LONG deltaY=0;
        int a=1;
        char *pBuchst;
        char *pRest;
        char winText[500];
        POINTL ptl;

        HPS hpsMem;
        BITMAPINFOHEADER2 bmpIH2;
        PBITMAPINFO2 pbmp2;
        char * chrBuff;
        PBYTE ptr;
        HDC hdcMem;
        HBITMAP hbm;
        ULONG ulCx, ulCy;
        SWP swp;
        RGB rgbTBFlyBackground;
        ULONG attrFound;

        hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);

        WinQueryWindowRect(hwnd, &rcl);

        GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULLHANDLE);

#if 0
        /* Original */
        if(g_fDisableFlyOverTransparency)
          WinFillRect(hps, &rcl,
                      rgbTBFlyBackground.bRed*65536
                      +rgbTBFlyBackground.bGreen*256
                      +rgbTBFlyBackground.bBlue);
#endif
        if(g_fDisableFlyOverTransparency)
          {
          /* Query the current background colour */
          if(WinQueryPresParam(hwnd,
                                PP_BACKGROUNDCOLOR,0,&attrFound,sizeof(rgbTBFlyBackground),
                                &rgbTBFlyBackground, QPF_NOINHERIT))
            WinFillRect(hps, &rcl,
                        rgbTBFlyBackground.bRed*65536
                        +rgbTBFlyBackground.bGreen*256
                        +rgbTBFlyBackground.bBlue);
          }

        if(!g_fDisableFlyOverShadow || !g_fDisableFlyOverTransparency)
          {
            /* Size of client */
            ulCx=rcl.xRight;
            ulCy=rcl.yTop;
            
            bmpIH2.cbFix=sizeof(BITMAPINFOHEADER2);
            bmpIH2.cx=ulCx;
            bmpIH2.cy=ulCy;
            bmpIH2.cPlanes=1;
            bmpIH2.cBitCount=8;
            bmpIH2.cbImage=(((ulCx*(1<<bmpIH2.cPlanes)*(1<<bmpIH2.cBitCount))+31)/32)*bmpIH2.cy;
            
            chrBuff=(char*)malloc(bmpIH2.cbImage+sizeof(BITMAPINFO2)+256*sizeof(RGB2));
            if(chrBuff)
              {
                pbmp2=(PBITMAPINFO2)chrBuff;
                memset(pbmp2, 0, sizeof(BITMAPINFO2)+256*sizeof(RGB2));
                ptr=chrBuff+sizeof(BITMAPINFO2)+256*sizeof(RGB2);
            
                pbmp2->cbFix=sizeof(BITMAPINFO2);
                pbmp2->cx=ulCx;
                pbmp2->cy=ulCy;
                pbmp2->cPlanes=1;
                pbmp2->cBitCount=8;
                pbmp2->cbImage=((pbmp2->cx+31)/32)*pbmp2->cy;
                pbmp2->ulCompression=BCA_UNCOMP;
                pbmp2->ulColorEncoding=BCE_RGB;
            
                hdcMem=DevOpenDC(WinQueryAnchorBlock(hwnd),OD_MEMORY,"*", 0L/*4L*/,
                                 (PDEVOPENDATA)NULLHANDLE/*pszData*/, NULLHANDLE);
                if(hdcMem) {
                  SIZEL sizel= {0,0};

                  hpsMem=GpiCreatePS(WinQueryAnchorBlock(hwnd), hdcMem, &sizel,
                                     PU_PELS|GPIT_MICRO|GPIA_ASSOC);
                  if(hpsMem)
                    {                 
                      hbm=GpiCreateBitmap(hpsMem, &bmpIH2, FALSE, NULL, pbmp2);
                      if(hbm) {
                        HPS hpsDesktop;
                        POINTL ptl[3]={0};
                        RGB2 *prgb2;
                        int a, r,g,b;
 
                        hpsDesktop=WinGetScreenPS(HWND_DESKTOP);
                        GpiSetBitmap(hpsMem, hbm);

                        if(!g_fDisableFlyOverTransparency)
                          {
                            ptl[0].x=0;
                            ptl[0].y=0;
                            ptl[1].x=0+ulCx;
                            ptl[1].y=0+ulCy;
                            ptl[2].x=0;
                            ptl[2].y=0;
                            
                            WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl[2], 1);
                            
                            if(GpiBitBlt(hpsMem, hpsDesktop, 3, ptl , ROP_SRCCOPY, BBO_IGNORE)==GPI_ERROR)
                              {
                              }
                            
                            if(GpiQueryBitmapBits(hpsMem, 0, ulCy, ptr, pbmp2)==GPI_ALTERROR)
                              {
                                
                              }
                            
                            prgb2=(RGB2*)(++pbmp2);
                            for(a=0;a<256; a++, prgb2++) {
                              r=210;
                              g=210;
                              b=180;
                              
                              b+=(prgb2->bBlue /5);
                              g+=(prgb2->bGreen /5);
                              r+=(prgb2->bRed /5);
                              
                              if(r>255)
                                r=255;
                              if(r<0)
                                r=0;
                              prgb2->bRed=r;
                              
                              if(g>255)
                                g=255;
                              if(g<0)
                                g=0;
                              prgb2->bGreen=g;
                              
                              if(b>255)
                                
                                b=255;
                              if(b<0)
                                b=0;
                              prgb2->bBlue=b;        
                            }/* for */

                            /* Blit to client */
                            if(GpiSetBitmapBits(hpsMem, 0, ulCy, ptr, --pbmp2)!=GPI_ALTERROR)
                              {
                                ptl[0].x=0;
                                ptl[0].y=0;
                                ptl[1].x=ulCx;
                                ptl[1].y=ulCy;
                                ptl[2].x=0;
                                ptl[2].y=0;
                                GpiBitBlt(hps, hpsMem, 3, ptl , ROP_SRCCOPY, BBO_IGNORE);
                              }
                            
                          }/* !g_fDisableFlyOverTranparency */
                        
                        if(!g_fDisableFlyOverShadow)
                          {
                            ptl[0].x=0;
                            ptl[0].y=0;
                            ptl[1].x=0+ulCx;
                            ptl[1].y=0+ulCy;
                            ptl[2].x=0+shadowDeltaX;
                            ptl[2].y=0-shadowDeltaY;
                        
                            WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl[2], 1);
                        
                            if(GpiBitBlt(hpsMem, hpsDesktop, 3, ptl , ROP_SRCCOPY, BBO_IGNORE)==GPI_ERROR)
                              {
                              }
                        
                            if(GpiQueryBitmapBits(hpsMem, 0, ulCy, ptr, pbmp2)==GPI_ALTERROR)
                              {
                                
                              }
                        
                            /* Create shadow colors */
                            prgb2=(RGB2*)(++pbmp2);
                            for(a=0;a<256; a++, prgb2++) {
                              r=-50;
                              g=-50;
                              b=-50;
                          
                              b+=prgb2->bBlue;
                              g+=prgb2->bGreen;
                              r+=prgb2->bRed;
                              if(r>255)
                                r=255;
                              if(r<0)
                                r=0;
                              prgb2->bRed=r;
                          
                              if(g>255)
                                g=255;
                              if(g<0)
                                g=0;
                              prgb2->bGreen=g;
                          
                              if(b>255)
                                b=255;
                              if(b<0)
                                b=0;
                              prgb2->bBlue=b;        
                            }
                        
                            ptr=chrBuff+sizeof(BITMAPINFO2)+256*sizeof(RGB2);
                            /* Blit shadow */
                            if(GpiSetBitmapBits(hpsMem, 0, ulCy, ptr, --pbmp2)!=GPI_ALTERROR)
                              {
                                /* Shadow at the bottom */
                                ptl[0].x=shadowDeltaX;
                                ptl[0].y=shadowDeltaY * -1;
                                ptl[1].x=ulCx+shadowDeltaX;
                                ptl[1].y=0;
                                WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl[0], 2);
                                ptl[2].x=0;
                                ptl[2].y=0;
                                GpiBitBlt(hpsDesktop, hpsMem, 3, ptl , ROP_SRCCOPY, BBO_IGNORE);
                            
                                /* Right shadow */
                                ptl[0].x=ulCx;
                                ptl[0].y=shadowDeltaY * -1;
                                ptl[1].x=ulCx+shadowDeltaX;
                                ptl[1].y=shadowDeltaY * -1 + ulCy;
                                WinMapWindowPoints(hwnd, HWND_DESKTOP, &ptl[0], 2);
                            
                                ptl[2].x=ulCx-shadowDeltaX;
                                ptl[2].y=0;
                            
                                GpiBitBlt(hpsDesktop, hpsMem, 3, ptl , ROP_SRCCOPY, BBO_IGNORE);
                              }
                          }/* g_fDisableFlyOverShadow */

                        GpiSetBitmap(hpsMem, NULLHANDLE);
                        GpiDeleteBitmap(hbm);
                        WinReleasePS(hpsDesktop);
                      }/* hbm */
                      GpiDestroyPS(hpsMem);
                    }/* hpsMem */
                  DevCloseDC(hdcMem);
                }/* if(hdcMem) */
                free(chrBuff);
              }/* if(chrBuff) */
            /* Client area painted */
          }

        WinQueryWindowText(hwnd, sizeof(winText), winText);
        ulLen=strlen(winText);
        
        pRest=winText;

        while((pBuchst=strchr(pRest,0xa))!=NULL){
          /* Get size of this line */
          GpiQueryTextBox(hps, pBuchst-pRest, pRest, TXTBOX_COUNT, aptlPoints);
          *pBuchst=0;
          ptl.x=3;
          ptl.y=2+rcl.yTop-a*(aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y);
          GpiCharStringAt(hps, &ptl, strlen(pRest), pRest);
          pBuchst++;
          pRest=pBuchst;
          a++;
        }
        GpiQueryTextBox(hps, strlen(pRest), pRest, TXTBOX_COUNT, aptlPoints);
        ptl.x=3;
        ptl.y=2+rcl.yTop-a*(aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y);
        GpiCharStringAt(hps, &ptl, strlen(pRest), pRest);
          
        WinEndPaint(hps);
        return MRFALSE;
      }
    default:
      break;
    }
  if(g_pfnwpOrgStaticProc)
    return g_pfnwpOrgStaticProc(hwnd,msg,mp1,mp2);
  return WinDefWindowProc(hwnd,msg,mp1,mp2);
}

static void setFlyOverText(HWND hwndBubbleWindow,HWND hwndBubbleClient, char * winText)
{
  POINTL ptl;
  POINTL aptlPoints[TXTBOX_COUNT];
  ULONG  ulLen;
  HPS hps;
  RECTL rcl;
  LONG   deltaX, deltaY;
  LONG cy=0;
  LONG cx=0;
  int a=1;
  char *pBuchst;
  char *pRest;


  if(!winText)
    return;
  
  ulLen=strlen(winText);
  if(!ulLen)
    return;

  WinSetWindowText(hwndBubbleClient, winText);
  
  /* Calculate text size in pixel */
  hps=WinGetPS(hwndBubbleClient);
  pRest=winText;
  while((pBuchst=strchr(pRest, 0xa))!=NULL) {
    a++;
    /* Get size of this line */
    GpiQueryTextBox(hps, pBuchst-pRest, pRest, TXTBOX_COUNT, aptlPoints);
    cx=(aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
        > cx 
        ? aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
        : cx);

    cy=(aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
        > cy 
        ? aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
        : cy) ;

    *pBuchst=0;
    pBuchst++;
    pRest=pBuchst;
    
  }

  GpiQueryTextBox(hps, strlen(pRest), pRest, TXTBOX_COUNT, aptlPoints);
  cx=(aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
      > cx 
      ? aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x 
      : cx);
          
  cy=(aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
      > cy 
      ? aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y
      : cy) ;

  WinReleasePS(hps);
 
  /* Calculate bubble positon and show bubble */
  WinQueryPointerPos(HWND_DESKTOP,&ptl);/*Query pointer position in the desktop window*/
  WinQueryWindowRect(HWND_DESKTOP,&rcl);/*Query desktop size*/

  deltaX=(cx+7+xVal+ptl.x 
          > rcl.xRight 
          ? -cx-xVal-xVal-7 
          : 0) ;
  deltaY=(cy*a+2+yVal+ptl.y 
          > rcl.yTop 
          ? -cy*a-2*yVal-7
          : 0) ;

  WinSetWindowPos(hwndBubbleWindow,
                  HWND_TOP,
                  ptl.x+xVal+deltaX, ptl.y+yVal+deltaY,
                  cx+8,
                  cy*a+2+2,
                  SWP_ZORDER | SWP_SIZE | SWP_MOVE /*| SWP_SHOW*/);
  WinShowWindow(hwndBubbleWindow,
                TRUE);
                  
}
static MRESULT EXPENTRY _bubbleHelpProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{

  MRESULT mrc;
  HPS hps;
  PWNDPARAMS pwp;
  RECTL rcl;
  static HWND hwndBubbleWindow;
  POINTL ptl;
  POINTL aptlPoints[TXTBOX_COUNT];

  switch(msg) {
  case WM_SETWINDOWPARAMS:
    {
      pwp=(PWNDPARAMS)mp1;
      if(pwp->fsStatus==WPM_TEXT) {
        /* The text changed */
        char *ptr;
        char *ptr2;

        if((ptr2=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
          free(ptr2); /* Free the old text */
        WinSetWindowPtr(hwnd,QWL_TEXTPTR, NULLHANDLE);

        if(strlen(pwp->pszText)) {
          /* Set the delay */
          if(strstr(pwp->pszText,"#delay")!=NULLHANDLE) {
            ptr=pwp->pszText;
            ptr+=strlen("#delay");
            WinSetWindowULong(hwnd,QWL_DELAY, atol(ptr));
            return (MRESULT)0;
          }
          /* Set the time to show the bubble */
          if(strstr(pwp->pszText,"#show")!=NULLHANDLE) {
            ptr=pwp->pszText;
            ptr+=strlen("#show");
            WinSetWindowULong(hwnd,QWL_SHOWTIME, atol(ptr));
            return (MRESULT)0;
          }
#if 0
          if(strstr(pwp->pszText,"#deltax")!=NULLHANDLE) {
            ULONG ulDelta;
            SHORT dx;
            ptr=pwp->pszText;
            ptr+=strlen("#deltax");
            dx=atoi(ptr);
            ulDelta=WinQueryWindowULong(hwnd, QWL_DELTAPOS) & 0x0000FFFF;
            WinSetWindowULong(hwnd,QWL_DELTAPOS, (ULONG)((LONG)dx));
            return (MRESULT)0;
          }

          if(strstr(pwp->pszText,"#deltay")!=NULLHANDLE) {
            ULONG ulDelta;
            ptr=pwp->pszText;
            ptr+=strlen("#deltay");
            ulDelta=WinQueryWindowULong(hwnd, QWL_DELTAPOS)& 0xFFFF0000;
            WinSetWindowULong(hwnd,QWL_DELTAPOS, (atol(ptr)&0x0000FFFF)+ulDelta);
            return (MRESULT)0;
          }
#endif
          /* There's text to save */
          if((ptr2=malloc(strlen(pwp->pszText)))!=NULLHANDLE) {
            strcpy(ptr2, pwp->pszText);
            WinSetWindowPtr(hwnd,QWL_TEXTPTR,ptr2);
            WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_DELAY_TIMER);  /* stop the running timer */
            WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_SHOW_TIMER);  /* stop the running timer */
            WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_CHECK_TIMER);
            if(hwndBubbleWindow){
              WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
              hwndBubbleWindow=NULLHANDLE;
            }
            /* Start a timer */
            WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_DELAY_TIMER,
                          WinQueryWindowULong(hwnd,QWL_DELAY)); /* New timer for delay */
            WinSetWindowULong(hwnd, QWL_SHOWBUBBLE,1);/* Mark that we have bubble text */
          }
        }
        else {
          /* No text */
            WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_DELAY_TIMER);  /* stop the running timer */
            WinSetWindowULong(hwnd, QWL_SHOWBUBBLE,0);/* Mouse left control. We need no bubble */
        }
        return (MRESULT)0;
      }/* WPM_TEXT */
      break;
    }
  case WM_TIMER:
    switch (SHORT1FROMMP(mp1))
      {
      case BUBBLE_DELAY_TIMER: /* Intervall timer */
        {
          HWND hwndBubbleClient;
          ULONG style=FCF_BORDER|FCF_NOBYTEALIGN;
          int deltaY=0;
          int deltaX=0;
          char * winText;
          char chrTBFlyFontName[100];
          ULONG  attrFound;
          ULONG  len;
          RGB rgb;
          SHORT dx;

#if 0
          deltaY=WinQueryWindowULong(hwnd, QWL_DELTAPOS)& 0x0000FFFF;
#endif

          WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_DELAY_TIMER);  /* stop the running timer */
          /*  we have to build a new information window  */
          if(hwndBubbleWindow){
            WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
            hwndBubbleWindow=NULLHANDLE;
          }
          /* Query the pointer position */
          WinQueryPointerPos(HWND_DESKTOP,&ptl);
          
          /* Create help window */
          if((hwndBubbleWindow=WinCreateStdWindow(HWND_DESKTOP,
                                                  0,
                                                  &style,
                                                  "_BUUBLECLIENT",/*WC_STATIC,*/
                                                  "The window",
                                                  SS_TEXT|DT_CENTER|DT_VCENTER,
                                                  NULLHANDLE,
                                                  400,
                                                  &hwndBubbleClient))==NULLHANDLE)
            return (MRESULT) 0;

          /* Set the font for the help */
          if(WinQueryPresParam(hwnd ,
                               PP_FONTNAMESIZE,0,&attrFound,sizeof(chrTBFlyFontName),
                               chrTBFlyFontName,QPF_NOINHERIT))
            WinSetPresParam(hwndBubbleClient,PP_FONTNAMESIZE,
                            sizeof(chrTBFlyFontName),
                            chrTBFlyFontName);
          /* Query the current background colour */
          if(WinQueryPresParam(hwnd,
                                PP_BACKGROUNDCOLOR,0,&attrFound,sizeof(rgb),
                                &rgb, QPF_NOINHERIT))
            WinSetPresParam(hwndBubbleClient,
                            PP_BACKGROUNDCOLOR,(ULONG)sizeof(rgb), &rgb );
            /* Query the current foreground colour */
          if(WinQueryPresParam(hwnd,
                               PP_FOREGROUNDCOLOR,0,&attrFound,sizeof(rgb),
                               &rgb,QPF_NOINHERIT))
            WinSetPresParam(hwndBubbleClient,
                            PP_FOREGROUNDCOLOR,(ULONG)sizeof(rgb), &rgb);          

          /* Set bubble text */
          if((winText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
            /*            WinSetWindowText(hwndBubbleClient,winText);*/
            setFlyOverText(hwndBubbleWindow, hwndBubbleClient, winText);
          else {
            WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
            hwndBubbleWindow=NULLHANDLE;
          }

#if 0
          /* Calculate text size in pixel */
          hps=WinBeginPaint(hwndBubbleClient,(HPS)NULL,(PRECTL)NULL);
          GpiQueryTextBox(hps,strlen(winText),winText,TXTBOX_COUNT,aptlPoints);
          WinEndPaint(hps);

          /* Calculate bubble positon and show bubble */
          WinQueryWindowRect(HWND_DESKTOP,&rcl);/* Query desktop size */

          deltaX=(aptlPoints[TXTBOX_BOTTOMRIGHT].x+7+xVal+ptl.x+deltaX 
            > rcl.xRight
                  ? -(aptlPoints[TXTBOX_BOTTOMRIGHT].x-(rcl.xRight-ptl.x)+7+7+xVal)
                  /*                 ? -(aptlPoints[TXTBOX_BOTTOMRIGHT].x-(rcl.xRight-ptl.x)+7+7+xVal)*/
 
                  /*? -(rcl.xRight-(aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x))*/
                  /*     ? -(aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x-xVal-xVal-7 )*/
            : deltaX ) ;  


          deltaY=(aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2+yVal+ptl.y 
            > rcl.yTop 
            ? deltaY-aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y-2*yVal-7
            : 0 );

          WinSetWindowPos(hwndBubbleWindow,
                          HWND_DESKTOP,
                          ptl.x+xVal+deltaX,ptl.y+yVal+deltaY,  
                          aptlPoints[TXTBOX_BOTTOMRIGHT].x-aptlPoints[TXTBOX_BOTTOMLEFT].x+8,
                          aptlPoints[TXTBOX_TOPLEFT].y-aptlPoints[TXTBOX_BOTTOMLEFT].y+2,
                          SWP_SIZE|SWP_MOVE|SWP_SHOW);
#endif

          WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_SHOW_TIMER, 
                        WinQueryWindowULong(hwnd,QWL_SHOWTIME)); /* New timer for delay */
          WinStartTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_CHECK_TIMER, (ULONG) 100); /* New timer for checking if mouse left */
          return (MRESULT) 0;
        }
        break;
      case BUBBLE_SHOW_TIMER: /* Show timer */
        WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_SHOW_TIMER);  /* stop the running timer */
        WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_CHECK_TIMER);  /* stop the running timer */
        if(hwndBubbleWindow){
          WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
          hwndBubbleWindow=NULLHANDLE;
        }
        return (MRESULT) 0;
      case BUBBLE_CHECK_TIMER: /* Check for mouse leaving the control */
        if(WinQueryWindowULong(hwnd,QWL_SHOWBUBBLE)==0) {
          if(hwndBubbleWindow){
            WinDestroyWindow(hwndBubbleWindow);/*  close the bubblewindow  */
            hwndBubbleWindow=NULLHANDLE;
          }
          WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_SHOW_TIMER);  /* stop the running timer */
          WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_CHECK_TIMER);  /* stop the running timer */
        }
        return (MRESULT) 0;
      default:
        break;
      }
    break;
  case WM_DESTROY:
    {
      char *ptrText;
      /* Free the memory allocated for the text */
      if((ptrText=(char*)WinQueryWindowPtr(hwnd,QWL_TEXTPTR))!=NULLHANDLE)
        free(ptrText);
      WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_DELAY_TIMER);  /* stop the running timer */
      WinStopTimer(WinQueryAnchorBlock(hwnd),hwnd,BUBBLE_SHOW_TIMER);  /* stop the running timer */
      break;
    }
  case WM_PAINT:
    {
      hps=WinBeginPaint(hwnd, NULLHANDLE, NULLHANDLE);
#if 0
      /* This is just to see if painting takes place. Nice for debugging. */
      WinQueryWindowRect(hwnd, &rcl);
      WinFillRect(hps,&rcl, CLR_RED);
#endif
      WinEndPaint(hps);
    return (MRESULT) FALSE;
    }
  
  default:
    break;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

/* This proc is a custom proc for the file dialog accepting only directories. */
MRESULT EXPENTRY dirDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) 
{
  switch(msg)
    {
    case WM_COMMAND:
      {
        if(SHORT1FROMMP(mp1)==DID_OK)
          {
            FILEDLG *fd;
            fd=(FILEDLG*)WinQueryWindowULong(hwnd, QWL_USER);
            WinQueryWindowText(WinWindowFromID(hwnd, 4096), CCHMAXPATH, fd->szFullFile);
            if(fd->szFullFile[strlen(fd->szFullFile)-1]=='\\')
              fd->szFullFile[strlen(fd->szFullFile)-1]=0;
            /*    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, fd->szFullFile, "", 123, MB_MOVEABLE|MB_OK); */
            fd->lReturn=DID_OK;
            WinDismissDlg(hwnd, DID_OK);
            return (MRESULT)FALSE;
          }
        break;
      }
    case WM_CONTROL:
      {
        if(SHORT2FROMMP(mp1)==CBN_EFCHANGE) {
          FILEDLG *fd;
          char text[CCHMAXPATH];

          fd=(FILEDLG*)WinQueryWindowULong(hwnd, QWL_USER);
          strncpy(text, fd->szFullFile,CCHMAXPATH);
          text[CCHMAXPATH-1]=0;
          text[strlen(text)-1]=0;
          WinSetWindowText(WinWindowFromID(hwnd, 4096), text);
        }
        break;
      }
    case WM_INITDLG:
      {
        MRESULT mr;
#if 0
        FILEDLG *fd;
        
        fd=(FILEDLG*)WinQueryWindowULong(hwnd, QWL_USER);
        if(fd->szFullFile[0]!=0)
          {
            WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, fd->szFullFile, "", 123, MB_MOVEABLE|MB_OK);
            if(fd->szFullFile[strlen(fd->szFullFile)]!='\\') {
              strcat(fd->szFullFile, "\\*.*");
         WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, fd->szFullFile, "", 123, MB_MOVEABLE|MB_OK);
              
            }
          }
#endif
        WinSendMsg(WinWindowFromID(hwnd, 4096), EM_SETTEXTLIMIT, MPFROMSHORT(CCHMAXPATH), 0L);
        mr=WinDefFileDlgProc(hwnd, msg, mp1, mp2);
        WinSetWindowText(WinWindowFromID(hwnd, 258),"");
        WinEnableWindow(WinWindowFromID(hwnd, DID_OK), TRUE);
        return mr;
      }
    default:
      break;
    }
  return WinDefFileDlgProc(hwnd, msg, mp1, mp2);
}


ULONG DRCtrlVersion(CHAR *name, ULONG numargs, RXSTRING args[],
                     CHAR *queuename, RXSTRING *retstr)
{

  retstr->strlength = 0;               /* set return value           */
  /* check arguments            */
  if (numargs > 0)
    return INVALID_ROUTINE;
  

  sprintf(retstr->strptr, "%s", DRDCTRL_VERSION);
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}


ULONG DRCtrlPickDirectory(CHAR *name, ULONG numargs, RXSTRING args[],
                     CHAR *queuename, RXSTRING *retstr)
{
  FILEDLG fd = { 0 };
  char chrTitle[255];

  retstr->strlength = 0;               /* set return value           */
  /* check arguments            */
  if (numargs > 2)
    return INVALID_ROUTINE;

  fd.cbSize = sizeof( fd );
  /* It's an centered 'Open'-dialog */
  fd.fl = FDS_CUSTOM|FDS_OPEN_DIALOG|FDS_CENTER;
  fd.pfnDlgProc=dirDialogProc;
  fd.usDlgId=IDDLG_DIRECTORY;
  fd.hMod=hModule;
  fd.pszTitle = "Find directory";

  if(numargs>1  && RXVALIDSTRING(args[1]))
    {
      /* Set the title of the file dialog */
      strncpy(chrTitle, args[1].strptr, sizeof(chrTitle));
      chrTitle[sizeof(chrTitle)-1]=0;
      fd.pszTitle = chrTitle;
    }
  if(numargs!=0 && RXVALIDSTRING(args[0]))
    {
      strncpy(fd.szFullFile, args[0].strptr, CCHMAXPATH-1);
      if(fd.szFullFile[strlen(fd.szFullFile)-1]!='\\')
        strcat(fd.szFullFile, "\\");
    }
#if 0
  strncpy(fd.szFullFile, globalData.chrMP3LibraryPath, CCHMAXPATH);
  strcat(fd.szFullFile, "\\");
#endif
  fd.szFullFile[CCHMAXPATH-1]=0;

  strcpy(retstr->strptr, "");
  if( WinFileDlg( HWND_DESKTOP, HWND_DESKTOP, &fd ) == NULLHANDLE )
    {
      /* WinFileDlg failed */

    }
  if( fd.lReturn == DID_OK )
    {
      sprintf(retstr->strptr, "%s", fd.szFullFile);
      /*WinSetWindowText( WinWindowFromID(hwnd,IDEF_MP3LIBRARY), fd.szFullFile );*/
    }

  retstr->strlength = strlen(retstr->strptr);

  return VALID_ROUTINE;
}


static HWND _getHWNDFromID(HWND hwndParent, USHORT id)
{
  PTIB ptib;
  PPIB ppip;
  /*  ULONG rc;*/
  HENUM henum;
  PID pid;
  TID tid;
  HWND hwnd=NULLHANDLE;


  if(NO_ERROR!=DosGetInfoBlocks(&ptib, &ppip))
      return NULLHANDLE;

  henum=WinBeginEnumWindows(hwndParent);
  while((hwnd=WinGetNextWindow(henum))!=NULLHANDLE)
    {
      if(WinQueryWindowUShort(hwnd, QWS_ID)==id) {
        WinQueryWindowProcess(hwnd,&pid, &tid);
        /* There's only one control with a given ID and a given parent in a DRDialog process */
        if(pid==ppip->pib_ulpid)
          break;
      }
    }
  WinEndEnumWindows(henum);

  return hwnd;
}

/*************************************************************************
* Function:  DRCtrlGetHWND                                               *
*                                                                        *
*            Gets the HWND of a dialog or control from the ID.           *
*                                                                        *
*                                                                        *
* Syntax:    call DRCtrlGetHWND hwnd, ID                                 *
*                                                                        *
* Params:    hwnd, ID, 1/0                                               *
*                                                                        *
* Return:    0: failure, 1: success                                      *
*                                                                        *
* Remarks:   If hwnd==0 it's assumed the dialog is a child of the        *
*            desktop.                                                    *
*                                                                        *
*************************************************************************/
ULONG DRCtrlGetHWND(CHAR *name, ULONG numargs, RXSTRING args[],
                         CHAR *queuename, RXSTRING *retstr)
{
  HWND hwndParent;

    retstr->strlength = 0;               /* set return value           */
  /* check arguments            */
  if (numargs != 2)
    return INVALID_ROUTINE;

  hwndParent=atol(args[0].strptr);
  if(!hwndParent)
    hwndParent=HWND_DESKTOP;
  
  sprintf(retstr->strptr, "%d", _getHWNDFromID(hwndParent, (USHORT)atol(args[1].strptr)));
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  DRCtrlSetParentFromHWND                                     *
*                                                                        *
*            Sets the parent<->child relationship of dialogs using       *
*            HWNDs.                                                      *
*                                                                        *
* Syntax:    call DRCtrlSetParentFromHWND hwnd, newParentHWND            *
*                                                                        *
* Params:    hwnd, newParentHWND, 1/0                                    *
*                                                                        *
* Return:    0: failure, 1: success                                      *
*                                                                        *
* Remarks:   The dialogs may be child dialogs of a dialog.               *
*            Use DRCtrlGetHWND() to query the HWND of a dialog.          *
*                                                                        *
*************************************************************************/
ULONG  DRCtrlSetParentFromHWND(CHAR *name, ULONG numargs, RXSTRING args[],
                               CHAR *queuename, RXSTRING *retstr)
{
  HWND hwndParent, hwnd;
  
  retstr->strlength = 0;               /* set return value           */
  /* check arguments            */
  if (numargs != 2)
    return INVALID_ROUTINE;
  
  hwnd=atol(args[0].strptr);
  hwndParent=atol(args[1].strptr);
  if(!hwnd || !hwndParent)
    return INVALID_ROUTINE;

  sprintf(retstr->strptr, "%d", WinSetParent(hwnd, hwndParent, TRUE));
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  DRCtrlSetParent                                             *
*                                                                        *
* Syntax:    call DRCtrlSetParent dialogID, newParentID                  *
*                                                                        *
* Params:    dialogID, newParentID, 1/0                                  *
*                                                                        *
* Return:    0: failure, 1: success                                      *
*                                                                        *
* Remarks:   The dialogs MUST be main dialogs. This means they're        *
*            childs of HWND_DESKTOP. If you want to change parents for   *
*            other dialogs use DRCtrlSetParentFromHWND().                *
*                                                                        *
*************************************************************************/
ULONG DRCtrlSetParent(CHAR *name, ULONG numargs, RXSTRING args[],
                         CHAR *queuename, RXSTRING *retstr)
{
  HWND hwndParent, hwnd;

 retstr->strlength = 0;               /* set return value           */
  /* check arguments            */
  if (numargs != 2)
    return INVALID_ROUTINE;

  hwndParent=_getHWNDFromID(HWND_DESKTOP, (USHORT)atol(args[1].strptr));
  hwnd=_getHWNDFromID(HWND_DESKTOP, (USHORT)atol(args[0].strptr));

  if(!hwnd || !hwndParent)
    return INVALID_ROUTINE;

#if 0
  sprintf(text, "%x  %x", hwnd, hwndParent);
  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,text, "",123, MB_MOVEABLE);
#endif


  sprintf(retstr->strptr, "%d", WinSetParent(hwnd, hwndParent, TRUE));
  retstr->strlength = strlen(retstr->strptr);

  /* Hilite titlebar of main dialog */
  hwnd=WinWindowFromID(hwndParent, FID_TITLEBAR);
  WinSendMsg(hwnd, TBM_SETHILITE, MPFROMSHORT(TRUE), 0L);
  WinSetWindowUShort(hwndParent,QWS_FLAGS, FF_ACTIVE|FF_DIALOGBOX);

#if 0
  /*  if(  WinSendMsg(hwnd, TBM_QUERYHILITE, 0L, 0L))
    DosBeep(500, 300);
    */
    WinSetWindowULong(HWND_DESKTOP,QWL_HWNDFOCUSSAVE, hwndParent);
  WinFocusChange(HWND_DESKTOP, hwndParent, FC_NOLOSEFOCUS|FC_NOLOSEACTIVE);
  /*  WinSetActiveWindow( HWND_DESKTOP, hwndParent, TRUE);*/
#endif

  return VALID_ROUTINE;
}

/*************************************************************************
* Function:  DRCtrlDropFuncs                                             *
*                                                                        *
* Syntax:    call DRCtrlDropFuncs                                        *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

ULONG DRCtrlDropFuncs(CHAR *name, ULONG numargs, RXSTRING args[],
                          CHAR *queuename, RXSTRING *retstr)
{
  INT     entries;                     /* Num of entries             */
  INT     j;                           /* Counter                    */

  if (numargs != 0)                    /* no arguments for this      */
    return INVALID_ROUTINE;            /* raise an error             */

  retstr->strlength = 0;               /* return a null string result*/

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++)
    RexxDeregisterFunction(RxFncTable[j]);

  return VALID_ROUTINE;                /* no error on call           */
}

#if 0
ULONG cwRexxCommandHandler(PRXSTRING prxCommand, PUSHORT pusFlag, PRXSTRING prxRetString)
{
  char text[100]={0};

    *pusFlag=RXSUBCOM_OK;

    if(prxCommand && prxCommand->strptr) {
      if(prxCommand->strlength>sizeof(text))
        memcpy(text,prxCommand->strptr, sizeof(text));
      else {
        memcpy(text,prxCommand->strptr, prxCommand->strlength);
        text[prxCommand->strlength]=0;
      }
      text[sizeof(text)-1]=0;
    }
  HlpWriteToTrapLog("%s %x \n", text, prxCommand);
   return 0;
}

LONG cwRexxExitHandler(LONG lExitNumber, LONG lSubfunction, PEXIT pexParameter)
{

  HlpWriteToTrapLog("lExitNumber: %d lSubfunction: %d\n", lExitNumber, lSubfunction);
  return RXEXIT_NOT_HANDLED;
}
#endif
/*************************************************************************
*                                                                        *
* Register the percent bar control with the calling PM process           *
*                                                                        *
*************************************************************************/

ULONG DRCtrlRegister(CHAR *name, ULONG numargs, RXSTRING args[],
                     CHAR *queuename, RXSTRING *retstr)
{
  CLASSINFO ci;
  if(WinQueryClassInfo(WinQueryAnchorBlock(HWND_DESKTOP),
                       (PSZ)WC_STATIC,
                       &ci))
    {
      /*      g_ulFrameDataOffset=ci.cbWindowData;*/
      g_pfnwpOrgStaticProc=ci.pfnWindowProc;
      
      /*      ulQWP_FCTRLDATA=g_ulFrameDataOffset;*/
     
#if 0 
      if (WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),
                           (PSZ)"wpFolder window",
                           fnwpCWFolderFrameProc,
                           //                                     ci.pfnWindowProc, 
                           ci.flClassStyle,
                           ci.cbWindowData + WIZ_ADDITIONAL_WND_DATA))
        {
          /* */
          
        }
#endif
    }
         
         
  WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"DRD_PERCENTBAR", _percentBarProc,0L,12);
  WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"DRD_BUBBLEHELP", _bubbleHelpProc,0L, BUBBLEHELP_ULONGS);
  /* */
  WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"_BUUBLECLIENT", _bubbleClientProc,0L, 0);
  WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"DRD_IMAGE", _imageProc,0L, IMAGE_BYTES);
  WinRegisterClass(WinQueryAnchorBlock(HWND_DESKTOP),"DRD_HISTOGRAM", _histogramProc,0L, IMAGE_BYTES);

  /*  HlpWriteToTrapLog("Register: %d\n",RexxRegisterSubcomExe("CWREXX", (PFN)cwRexxCommandHandler, NULLHANDLE));*/
  /*  HlpWriteToTrapLog("Register Exit: %d\n",RexxRegisterExitExe("CWREXX", (PFN)cwRexxExitHandler, NULLHANDLE));*/

  return VALID_ROUTINE;                /* no error on call           */
}


/*************************************************************************
* Function:  DRCtrlLoadFuncs                                             *
*                                                                        *
* Syntax:    call DRCtrlLoadFuncs                                        *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

ULONG DRCtrlLoadFuncs(CHAR *name, ULONG numargs, RXSTRING args[],
                           CHAR *queuename, RXSTRING *retstr)
{
  INT    entries;                      /* Num of entries             */
  INT    j;                            /* Counter                    */

  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */

  if (numargs > 0)
    return INVALID_ROUTINE;

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0; j < entries; j++) {
    RexxRegisterFunctionDll(RxFncTable[j],
          DRDCTRL_DLL_NAME, RxFncTable[j]);
  }

  return VALID_ROUTINE;
}

int _CRT_init(void);
#ifdef STATIC_LINK
  void  _CRT_term();
#endif


ULONG _System _DLL_InitTerm(ULONG modHandle, ULONG ulFlag)
{
  /* Initialization */
  if(!ulFlag) {
    hModule=modHandle;
    if(_CRT_init()==-1)
      return 0UL;
  }
  else{
#ifdef STATIC_LINK
    _CRT_term();
#endif
  }
  return 1UL;/* Always succeed */
}










