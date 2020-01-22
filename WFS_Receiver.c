 /*===============================================================================================================================

	Thorlabs DataSocket Data Receiver Application for WaveFrontSensor

	This LabWindows/CVI example illustrates using DataSocket for reading in 
	Thorlabs WaveFrontSensor results.
  
	Start WFS.exe and open a connection to a DataSocket server.
	Enable data transfer for basic and/or wavefront results. Then open this
	DataSocket Receiver application and connect it to the same DataSocket server.
  
	The Writer, Server, and Reader can all be on the same PC or different PCs.
  
	Be sure to configure the DataSocket server to enable read and write access
	to the involved PCs. Allow DataSocket communication through firewalls.

	Source file 'WFS_Receiver.c'

	Date:          Dec-10-2009
	Software-Nr:   N/A
	Version:       3.0
	Copyright:     Copyright(c) 2009, Thorlabs GmbH (www.thorlabs.com)
	Author:        Egbert Krause (ekrause@thorlabs.com)

	Changelog:     Oct-23-2007 -> V1.0 first release, fits to GUI V1.0
						Nov-07-2007 -> V1.1 fits to GUI V1.1
						Dec-06-2007 -> V1.2 without changes
						Jun-24-2009 -> V1.4 centroid data added
						Dec-07-2009 -       WFS_Drv.h included, #defines deleted
						Dec-07-2009 -       centroid data type changed to float
						Dec-07-2009 -       printout of centroid data corrected
						Dec-07-2009 -> V2.0 
						Dec-10-2010 -> V3.0 minor changes
						May-05-2017 -> V4.0 compiled with new WFS.h to support WFS30, WFS40
						
	Disclaimer:

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

===============================================================================================================================*/

/*===============================================================================================================================
  Include Files
  Note: This example program uses LabWindows/CVI functions that may not be available from other compilers
===============================================================================================================================*/
#include <cvidef.h>
#include <cvirte.h>     // Needed if linking in external compiler; harmless otherwise
#include <cviauto.h>
#include <utility.h>
#include <userint.h>
#include <ansi_c.h>
#include "WFS_Receiver.h"
#include "dataskt.h"
#include "WFS.h"    // header file of Thorlabs WFS instrument driver



/*===============================================================================================================================
  Defines
===============================================================================================================================*/
// all required defines in WFS_Drv.h


/*===============================================================================================================================
  Data type definitions
===============================================================================================================================*/
typedef struct
{
	char              instrument_name[WFS_BUFFER_SIZE];
	char              serial_number[WFS_BUFFER_SIZE];
	float             wavelength;
	int               wf_unit;
	int               wf_calc_in_pupil;
	int               wavefront_type;
	int               zernike_orders;
	int               zernike_modes; 
} wfs_setup_t;
	
	
typedef struct
{
	float             spots_centroid_x_mm;
	float             spots_centroid_y_mm;
	float             spots_diameter_x_mm;
	float             spots_diameter_y_mm;

	float             wf_diff;          
	float             wf_rms;           
	float             weighted_wf_rms;  

	float             fourier_M;        
	float             fourier_J0;       
	float             fourier_J45;      

	float             optometric_sphere;
	float             optometric_cyl;   
	float             optometric_axis_deg;
	
	float             roc_mm;
		
	float             zernike_orders_rms[MAX_ZERNIKE_ORDERS+1];
	float             zernike[MAX_ZERNIKE_MODES+1];

	float             fit_err_mean_arcmin;
	float             fit_err_stdev_arcmin;
	
	int               spots_x; // used microlens spots
	int               spots_y; // used microlens spots 
	float             wavefront[MAX_SPOTS_Y][MAX_SPOTS_X]; // entire wavefront array
	
	int               centroids_x; // centroid positions in array, may be MAX_SPOTS_X+1 !
	int               centroids_y; // centroid positions in array, may be MAX_SPOTS_Y+1 !
	float             centroid_x[MAX_SPOTS_Y][MAX_SPOTS_X]; // centroid x-coordinates
	float             centroid_y[MAX_SPOTS_Y][MAX_SPOTS_X]; // centroid y-coordinates
} wfs_result_t;


/*===============================================================================================================================
  Function Prototypes
===============================================================================================================================*/
void CVICALLBACK DSCallback (DSHandle dsHandle, int event, void *callbackData); // callback function for the DataSocket
void update_param (void);
void save_wavefront_data (void *fp);


/*===============================================================================================================================
  Global Variables
===============================================================================================================================*/
wfs_setup_t     wfs_setup;
wfs_result_t    wfs_result;

static int panelHandle;
static DSHandle sDSHandle = 0;

int save_status;

/*===============================================================================================================================
  Code
===============================================================================================================================*/
int main (int argc, char *argv[])
{
	 if (InitCVIRTE (0, argv, 0) == 0)
		  return -1;
	 if ((panelHandle = LoadPanel (0, "WFS_Receiver.uir", PANEL)) < 0)
		  return -1;

	 // dummy data
	 // wfs_setup
	 //strcpy(wfs_setup.instrument_name, "WFS20-5C Wavefront Sensor");
	 //strcpy(wfs_setup.serial_number, "M00291303");
	 //wfs_setup.wavelength = 633.000;
	 //wfs_setup.wf_unit = 0;
	 //wfs_setup.wf_calc_in_pupil = 0;
	 //wfs_setup.wavefront_type = 0;
	 //wfs_setup.zernike_orders = 4;
	 //wfs_setup.zernike_modes = 15;
	 //wfs_result
	 //wfs_result.spots_centroid_x_mm = 0.210;
	 //wfs_result.spots_centroid_y_mm = 0.064;
	 //wfs_result.spots_diameter_x_mm = 3.396;
	 //wfs_result.spots_diameter_y_mm = 3.466;
	 //wfs_result.wf_diff = 5.076;
	 //wfs_result.wf_rms = 1.100;
	 //wfs_result.weighted_wf_rms = 0.977;
	 //wfs_result.roc_mm = 562.724;
	 //wfs_result.zernike[1] = 0.819;
	 //wfs_result.zernike[2] = -0.052;
	 //wfs_result.zernike[3] = -0.024;
	 //wfs_result.zernike[4] = -0.030;
	 //wfs_result.zernike[5] = -0.755;
	 //wfs_result.zernike[6] = 0.022;
	 //wfs_result.zernike[7] = 0.000;
	 //wfs_result.zernike[8] = 0.004;
	 //wfs_result.zernike[9] = 0.027;
	 //wfs_result.zernike[10] = -0.008;
	 //wfs_result.zernike[11] = -0.055;
	 //wfs_result.zernike[12] = -0.003;
	 //wfs_result.zernike[13] = 0.002;
	 //wfs_result.zernike[14] = 0.003;
	 //wfs_result.zernike[15] = 0.001;
	 
	 DisplayPanel (panelHandle);
	 RunUserInterface ();
	 return 0;
}


/*===============================================================================================================================
	Ring control changed - update the target text box 
===============================================================================================================================*/
int CVICALLBACK OnSourceRing (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	 char URL[500];
	 
	 switch (event)
	 {
		  case EVENT_COMMIT:
				GetCtrlVal(panelHandle, PANEL_RING, URL);
				SetCtrlVal(panelHandle, PANEL_SOURCE, URL);
				break;
	 }
	 return 0;
}



/*===============================================================================================================================
	ConnectAutoUpdate pressed 
===============================================================================================================================*/
int CVICALLBACK OnConnectAuto (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	 char URL[500];
	 
	 switch (event)
	 {
		  case EVENT_COMMIT: 
				if (sDSHandle)
				{
					 DS_DiscardObjHandle(sDSHandle); 
					 sDSHandle = 0;
				}
				GetCtrlVal (panelHandle, PANEL_SOURCE, URL);
				if(DS_Open (URL, DSConst_ReadAutoUpdate, DSCallback, NULL, &sDSHandle) == 0)
				{
					SetCtrlAttribute(panelHandle,PANEL_CONNECT_AUTO,ATTR_DIMMED,1);
					SetCtrlAttribute(panelHandle,PANEL_CONNECT_MANUAL,ATTR_DIMMED,1);
					SetCtrlAttribute(panelHandle,PANEL_UPDATE,ATTR_DIMMED,1);
					SetCtrlAttribute(panelHandle,PANEL_DISCONNECT,ATTR_DIMMED,0);
					SetCtrlAttribute(panelHandle,PANEL_SAVE_DATA,ATTR_DIMMED,1);
				}
				break;
	 }
	 return 0;
}


/*===============================================================================================================================
	ConnectManual button is pressed
===============================================================================================================================*/
int CVICALLBACK OnConnectManual (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)         
{   
	 char URL[500];
	 
	 switch (event)
	 {                                                           
		  case EVENT_COMMIT: 
				if (sDSHandle)
				{
					 DS_DiscardObjHandle(sDSHandle); 
					 sDSHandle = 0;
				}
				GetCtrlVal (panelHandle, PANEL_SOURCE, URL);                     
				if(DS_Open (URL, DSConst_Read, DSCallback, NULL, &sDSHandle) == 0)
				{
					SetCtrlAttribute(panelHandle,PANEL_CONNECT_AUTO,ATTR_DIMMED,1);
					SetCtrlAttribute(panelHandle,PANEL_CONNECT_MANUAL,ATTR_DIMMED,1);
					SetCtrlAttribute(panelHandle,PANEL_UPDATE,ATTR_DIMMED,0);
					SetCtrlAttribute(panelHandle,PANEL_DISCONNECT,ATTR_DIMMED,0);
					SetCtrlAttribute(panelHandle,PANEL_SAVE_DATA,ATTR_DIMMED,0);
				}
				break;                                                  
	 }                                                           
	 return 0;                                                       
}                                                                   


/*===============================================================================================================================
	Update button is pressed - call DS_Update
===============================================================================================================================*/
int CVICALLBACK OnUpdate (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	 switch (event)
	 {
		  case EVENT_COMMIT:  
				DS_Update (sDSHandle);
				break;
	 }
	 return 0;
}


/*===============================================================================================================================
	Quit button is pressed - Close and Quit
===============================================================================================================================*/
int CVICALLBACK QuitBtn (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	 switch (event)
	 {
		  case EVENT_COMMIT:
				if (sDSHandle)
				{
					 DS_DiscardObjHandle (sDSHandle); 
					 sDSHandle = 0;
				}
				QuitUserInterface (0);
				break;
	 }
	 return 0;
}



/*===============================================================================================================================
	callback that fires whenever the data or status is updated on the DataSocket server
===============================================================================================================================*/
void CVICALLBACK DSCallback (DSHandle dsHandle, int event, void *pcallbackData)
{
	 static int  function_active = 0;
	 HRESULT     hr = S_OK;
	 char        message[WFS_BUFFER_SIZE];
	 int         enable_beep;
	 
	 // prevent multiple calls of this callback function
	 if(function_active == 1)
		 return;
	 
	 function_active = 1;   
	 
	 switch (event)
	 {
		  case DS_EVENT_DATAUPDATED:
				if (sDSHandle)
				{
					// receive some instrument settings from DataSocket server
					hr = DS_GetAttrValue (dsHandle, "Instrument_Name",                  CAVT_CSTRING, &wfs_setup.instrument_name,     WFS_BUFFER_SIZE,  0, 0);
					hr = DS_GetAttrValue (dsHandle, "Serial_Number",                    CAVT_CSTRING, &wfs_setup.serial_number,       WFS_BUFFER_SIZE,  0, 0);
					
					hr = DS_GetAttrValue (dsHandle, "Wavelength",                       CAVT_FLOAT, &wfs_setup.wavelength,            sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Wavefront_Unit_Um_Waves",          CAVT_SHORT, &wfs_setup.wf_unit,               sizeof(short),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Wavefront_Data_EntireArea_Pupil",  CAVT_SHORT, &wfs_setup.wf_calc_in_pupil,      sizeof(short),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Wavefront_Type_Meas_Recon_Diff",   CAVT_SHORT, &wfs_setup.wavefront_type,        sizeof(short),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Zernike_Orders",                   CAVT_SHORT, &wfs_setup.zernike_orders,        sizeof(short),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Zernike_Modes",                    CAVT_SHORT, &wfs_setup.zernike_modes,         sizeof(short),    0, 0);
		
		
					// receive some measurement results from DataSocket server
					hr = DS_GetAttrValue (dsHandle, "Beam_Center_X",                    CAVT_FLOAT, &wfs_result.spots_centroid_x_mm,   sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Beam_Center_Y",                    CAVT_FLOAT, &wfs_result.spots_centroid_y_mm,   sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Beam_Diameter_X",                  CAVT_FLOAT, &wfs_result.spots_diameter_x_mm,   sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Beam_Diameter_Y",                  CAVT_FLOAT, &wfs_result.spots_diameter_y_mm,   sizeof(float),    0, 0);

					hr = DS_GetAttrValue (dsHandle, "Wavefront_PV",                     CAVT_FLOAT, &wfs_result.wf_diff,               sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Wavefront_RMS",                    CAVT_FLOAT, &wfs_result.wf_rms,                sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Wavefront_wRMS",                   CAVT_FLOAT, &wfs_result.weighted_wf_rms,       sizeof(float),    0, 0);

					// Fourier notation
					hr = DS_GetAttrValue (dsHandle, "Fourier_M",                        CAVT_FLOAT, &wfs_result.fourier_M,             sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Fourier_J0",                       CAVT_FLOAT, &wfs_result.fourier_J0,            sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Fourier_J45",                      CAVT_FLOAT, &wfs_result.fourier_J45,           sizeof(float),    0, 0);
	
					// Optimetric values
					hr = DS_GetAttrValue (dsHandle, "Optometric_Sphere",                CAVT_FLOAT, &wfs_result.optometric_sphere,     sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Optometric_Cylinder",              CAVT_FLOAT, &wfs_result.optometric_cyl,        sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Optometric_Axis",                  CAVT_FLOAT, &wfs_result.optometric_axis_deg,   sizeof(float),    0, 0);
	
					// Radius of curvature RoC
					hr = DS_GetAttrValue (dsHandle, "RoC_mm",                           CAVT_FLOAT, &wfs_result.roc_mm,               sizeof(float),    0, 0);
	
					// RMS Wavefront variations (Summarized Zernikes by orders)
					hr = DS_GetAttrValue (dsHandle, "Wavefront_Variations",             CAVT_FLOAT | CAVT_ARRAY, &wfs_result.zernike_orders_rms,  sizeof(float) * (wfs_setup.zernike_orders+1), 0, 0); // index 0 ... instr_setup.zernike_orders
	
					// Zernike array
					hr = DS_GetAttrValue (dsHandle, "Zernike",                          CAVT_FLOAT | CAVT_ARRAY, &wfs_result.zernike,  sizeof(float) * (wfs_setup.zernike_modes+1), 0, 0); // index 0 ... instr_setup.zernike_modes (0 is dummy)

					// Fit error
					hr = DS_GetAttrValue (dsHandle, "Fit_Error_Mean_Arcmin",            CAVT_FLOAT, &wfs_result.fit_err_mean_arcmin,   sizeof(float),    0, 0);
					hr = DS_GetAttrValue (dsHandle, "Fit_Error_StdDev_Arcmin",          CAVT_FLOAT, &wfs_result.fit_err_stdev_arcmin,  sizeof(float),    0, 0);

					// Wavefront
					hr = DS_GetAttrValue (dsHandle, "Wavefront_Spots_X",                CAVT_SHORT, &wfs_result.spots_x,               sizeof(short),    0, 0); // used array elements
					hr = DS_GetAttrValue (dsHandle, "Wavefront_Spots_Y",                CAVT_SHORT, &wfs_result.spots_y,               sizeof(short),    0, 0); // used array elements
					hr = DS_GetAttrValue (dsHandle, "Wavefront",                        CAVT_FLOAT | CAVT_ARRAY, &wfs_result.wavefront, sizeof(float) * (MAX_SPOTS_Y * MAX_SPOTS_X), 0, 0); // array dimensions!; 1st dimension = y; NaN indicates undefined point

					// Centroid data in pixels
					hr = DS_GetAttrValue (dsHandle, "Centroids_X",                      CAVT_SHORT, &wfs_result.centroids_x,               sizeof(short),    0, 0); // used array elements
					hr = DS_GetAttrValue (dsHandle, "Centroids_Y",                      CAVT_SHORT, &wfs_result.centroids_y,               sizeof(short),    0, 0); // used array elements
					hr = DS_GetAttrValue (dsHandle, "Centroid_Pos_X",                   CAVT_FLOAT | CAVT_ARRAY, &wfs_result.centroid_x, sizeof(double) * (MAX_SPOTS_Y * MAX_SPOTS_X), 0, 0); // 1st dimension = y; NaN indicates undefined point
					hr = DS_GetAttrValue (dsHandle, "Centroid_Pos_Y",                   CAVT_FLOAT | CAVT_ARRAY, &wfs_result.centroid_y, sizeof(double) * (MAX_SPOTS_Y * MAX_SPOTS_X), 0, 0); // 1st dimension = y; NaN indicates undefined point
					
					// display values
					update_param();

					// save data
					// make file pointer
					FILE *fp;
					save_status = 1;
					fp = fopen("wavefront_data.csv", "w");
					// start outputting data
					save_wavefront_data(fp);
					// close file
					fclose(fp);
					// make point to null
					fp = NULL;
					
					// enable beep?
					GetCtrlVal (panelHandle, PANEL_ENABLE_BEEP, &enable_beep);
					if(enable_beep)
						  Beep();
				}           
				break;
				
		  case DS_EVENT_STATUSUPDATED: 
				if (sDSHandle)
				{
					 hr = DS_GetLastMessage (dsHandle, message, WFS_BUFFER_SIZE);
					 if (SUCCEEDED(hr)) 
					 {
						SetCtrlVal (panelHandle, PANEL_STATUS, message);
						if(strstr(message, "Error: Not Connected"))
						{
							// do disconnect in case of error
							DS_DiscardObjHandle(sDSHandle); 
							sDSHandle = 0;
							
							SetCtrlAttribute(panelHandle,PANEL_CONNECT_AUTO,ATTR_DIMMED,0);
							SetCtrlAttribute(panelHandle,PANEL_CONNECT_MANUAL,ATTR_DIMMED,0);
							SetCtrlAttribute(panelHandle,PANEL_UPDATE,ATTR_DIMMED,1);
							SetCtrlAttribute(panelHandle,PANEL_DISCONNECT,ATTR_DIMMED,1);
							SetCtrlAttribute(panelHandle,PANEL_SAVE_DATA,ATTR_DIMMED,1);
							
							// save data
							// make file pointer
							FILE *fp;
							save_status = -1;
							fp = fopen("wavefront_data.csv", "w");
							// start outputting data
							save_wavefront_data(fp);
							// close file
							fclose(fp);
							// make point to null
							fp = NULL;
						} 
					 }  
				}
				break;
	 }
	 
	 function_active = 0;
	 
	 return;
}


/*===============================================================================================================================
	Disconnect button is pressed
===============================================================================================================================*/
int CVICALLBACK OnDisconnect (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	 switch (event)
	 {
		  case EVENT_COMMIT:
				SetCtrlVal (panelHandle, PANEL_STATUS, "Unconnected."); 
				if (sDSHandle)
				{
					DS_DiscardObjHandle(sDSHandle); 
					sDSHandle = 0;
				
					SetCtrlAttribute(panelHandle,PANEL_CONNECT_AUTO,ATTR_DIMMED,0);
					SetCtrlAttribute(panelHandle,PANEL_CONNECT_MANUAL,ATTR_DIMMED,0);
					SetCtrlAttribute(panelHandle,PANEL_UPDATE,ATTR_DIMMED,1);
					SetCtrlAttribute(panelHandle,PANEL_DISCONNECT,ATTR_DIMMED,1);
					SetCtrlAttribute(panelHandle,PANEL_SAVE_DATA,ATTR_DIMMED,1);
				}
				break;
	 }
	 return 0;
}



/*===============================================================================================================================
	SaveData button is pressed
===============================================================================================================================*/
int CVICALLBACK OnSaveData (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	// make file pointer
	FILE *fp;
	switch (event)
	{
		case EVENT_COMMIT:
			fp = fopen("wavefront_data.csv", "w");
			// start outputting data
			save_status = 2;
			save_wavefront_data(fp);
			// close file
			fclose(fp);
			// make point to null
			fp = NULL;
			break;
	}
	return 0;
}


/*===============================================================================================================================
	Display received parameters
===============================================================================================================================*/
void update_param (void)
{   
	int         i,j,k;
	char        buf[200];

	ResetTextBox (panelHandle, PANEL_TEXTBOX, "");
  
	// display WFS settings in textbox
	sprintf(buf, "Instrument_Name               %s",       wfs_setup.instrument_name);           InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Serial_Number                 %s",       wfs_setup.serial_number);             InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	
	InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, "\n*** WFS_Settings ***");
	
	sprintf(buf, "Wavelength                    %12.3f",   wfs_setup.wavelength);                InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Wavefront_Unit_Um_Waves         %10d",   wfs_setup.wf_unit);                   InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Wavefront_Data_EntireArea_Pupil %10d",   wfs_setup.wf_calc_in_pupil);          InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Wavefront_Type_Meas_Recon_Diff  %10d",   wfs_setup.wavefront_type);            InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Zernike_Modes                   %10d",   wfs_setup.zernike_modes);             InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Zernike_Orders                  %10d",   wfs_setup.zernike_orders);            InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	
	// display WFS results in textbox
	InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, "\n*** WFS_Results ***");
	sprintf(buf, "Beam_Center_X                 %12.3f",   wfs_result.spots_centroid_x_mm);      InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Beam_Center_Y                 %12.3f",   wfs_result.spots_centroid_y_mm);      InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Beam_Diameter_X               %12.3f",   wfs_result.spots_diameter_x_mm);      InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Beam_Diameter_Y               %12.3f",   wfs_result.spots_diameter_y_mm);      InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);

	sprintf(buf, "Wavefront_PV                  %12.3f",   wfs_result.wf_diff);                  InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Wavefront_RMS                 %12.3f",   wfs_result.wf_rms);                   InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Wavefront_wRMS                %12.3f",   wfs_result.weighted_wf_rms);          InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);

	sprintf(buf, "Fourier_M                     %12.3f",   wfs_result.fourier_M);                InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Fourier_J0                    %12.3f",   wfs_result.fourier_J0);               InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Fourier_J45                   %12.3f",   wfs_result.fourier_J45);              InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);

	sprintf(buf, "Optometric_Sphere             %12.3f",   wfs_result.optometric_sphere);        InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Optometric_Cylinder           %12.3f",   wfs_result.optometric_cyl);           InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Optometric_Axis               %12.3f",   wfs_result.optometric_axis_deg);      InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);

	sprintf(buf, "RoC_mm                        %12.3f",   wfs_result.roc_mm);                   InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	
	for(i=0;i<=wfs_setup.zernike_orders;i++)
	{
		sprintf(buf, "WF_Variations[%2d]         %16.3f",   i,wfs_result.zernike_orders_rms[i]);  InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	}
	
	for(i=1;i<=wfs_setup.zernike_modes;i++)
	{  
		sprintf(buf, "Zernike[%2d]               %16.3f",   i,wfs_result.zernike[i]);             InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	}
	
	sprintf(buf, "Fit_Error_Mean_Arcmin         %12.3e",   wfs_result.fit_err_mean_arcmin);      InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Fit_Error_StdDev_Arcmin       %12.3e",   wfs_result.fit_err_stdev_arcmin);     InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	

	InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, "\n*** WFS_Wavefront_Array (first 5x5 elements) ***");
	sprintf(buf, "Spots_x                       %12d",     wfs_result.spots_x);                  InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Spots_y                       %12d",     wfs_result.spots_y);                  InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	for(i=0;i<5;i++)
	{   
		buf[0] = '\0';
		k = 0;
		for(j=0;j<5;j++)
			k += sprintf(buf + k, " %8.3f", wfs_result.wavefront[i][j]); // 1st dimension = y; NaN indicates undefined point
		InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	}      
	
	
	InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, "\n*** WFS_CentroidX_Array in pixels (first 5x5 elements) ***");
	sprintf(buf, "Centroids_x                   %12d",     wfs_result.centroids_x);                  InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	sprintf(buf, "Centroids_y                   %12d",     wfs_result.centroids_y);                  InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	for(i=0;i<5;i++)
	{
		buf[0] = '\0';
		k = 0;
		for(j=0;j<5;j++)
			k += sprintf(buf + k, " %8.3f", wfs_result.centroid_x[i][j]); // 1st dimension = y; NaN indicates undefined point
		InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	}
	
	InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, "\n*** WFS_CentroidY_Array in pixels (first 5x5 elements) ***");
	for(i=0;i<5;i++)
	{
		buf[0] = '\0';
		k = 0;
		for(j=0;j<5;j++)
			k += sprintf(buf + k, " %8.3f", wfs_result.centroid_y[i][j]); // 1st dimension = y; NaN indicates undefined point
		InsertTextBoxLine (panelHandle, PANEL_TEXTBOX, -1, buf);
	}

}


/*===============================================================================================================================
	Display received parameters
===============================================================================================================================*/
void save_wavefront_data (void *fp)
{
	int i;
	
	fprintf(fp, "%s,%d\n\n", "Save_Status(-1_Fail;1_Auto;2_Manual)", save_status);	
	fprintf(fp, "%s,%s\n", "Instrument_Name", wfs_setup.instrument_name);
	fprintf(fp, "%s,%s\n", "Serial_Number", wfs_setup.serial_number);
	fprintf(fp, "\n%s\n", "*** WFS_Settings ***");
	fprintf(fp, "%s,%f\n", "Wavelength", wfs_setup.wavelength);
	fprintf(fp, "%s,%d\n", "Wavefront_Unit_Um_Waves", wfs_setup.wf_unit);
	fprintf(fp, "%s,%d\n", "Wavefront_Data_EntireArea_Pupil", wfs_setup.wf_calc_in_pupil);
	fprintf(fp, "%s,%d\n", "Wavefront_Tpye_Meas_Recon_Diff", wfs_setup.wavefront_type);
	fprintf(fp, "%s,%d\n", "Zernike_Modes", wfs_setup.zernike_modes);
	fprintf(fp, "%s,%d\n", "Zernike_Orders", wfs_setup.zernike_orders);
	fprintf(fp, "\n%s\n", "*** WFS_Results ***");
	fprintf(fp, "%s,%f\n", "Beam_Center_X", wfs_result.spots_centroid_x_mm);
	fprintf(fp, "%s,%f\n", "Beam_Center_Y", wfs_result.spots_centroid_y_mm);
	fprintf(fp, "%s,%f\n", "Beam_Diameter_X", wfs_result.spots_diameter_x_mm);
	fprintf(fp, "%s,%f\n", "Beam_Diameter_Y", wfs_result.spots_diameter_y_mm);
	fprintf(fp, "%s,%f\n", "Wavefront_PV", wfs_result.wf_diff);
	fprintf(fp, "%s,%f\n", "Wavefront_RMS", wfs_result.wf_rms);
	fprintf(fp, "%s,%f\n", "Wavefront_wRMS", wfs_result.weighted_wf_rms);
	fprintf(fp, "%s,%f\n", "Roc_mm", wfs_result.roc_mm);
	for(i=1;i<=wfs_setup.zernike_modes;i++)
	{
		fprintf(fp, "%s[%2d],%f\n", "Zernike", i, wfs_result.zernike[i]);
	}
}
/*===============================================================================================================================
	End of source file
===============================================================================================================================*/

