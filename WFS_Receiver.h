/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1
#define  PANEL_SOURCE                     2       /* control type: string, callback function: (none) */
#define  PANEL_RING                       3       /* control type: ring, callback function: OnSourceRing */
#define  PANEL_STATUS                     4       /* control type: string, callback function: (none) */
#define  PANEL_TEXTBOX                    5       /* control type: textBox, callback function: (none) */
#define  PANEL_QUITBTN                    6       /* control type: command, callback function: QuitBtn */
#define  PANEL_CONNECT_MANUAL             7       /* control type: command, callback function: OnConnectManual */
#define  PANEL_UPDATE                     8       /* control type: command, callback function: OnUpdate */
#define  PANEL_DISCONNECT                 9       /* control type: command, callback function: OnDisconnect */
#define  PANEL_CONNECT_AUTO               10      /* control type: command, callback function: OnConnectAuto */
#define  PANEL_ENABLE_BEEP                11      /* control type: radioButton, callback function: (none) */
#define  PANEL_SAVE_DATA                  12      /* control type: command, callback function: OnSaveData */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK OnConnectAuto(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnConnectManual(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDisconnect(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSaveData(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSourceRing(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnUpdate(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK QuitBtn(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif