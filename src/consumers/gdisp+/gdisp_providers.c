/*

$Id: gdisp_providers.c,v 1.15 2006-11-08 21:31:12 esteban Exp $

-----------------------------------------------------------------------

GDISP+ - Graphic Tool based on GTK+,
         for being used upon the generic Transport Sampling Protocol.

Copyright (c) 2003 - Euskadi.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

-----------------------------------------------------------------------

Project   : TSP
Maintainer: tsp@astrium.eads.net
Component : Graphic Tool

-----------------------------------------------------------------------

Purpose   : Graphic Tool based on GTK+ that provide several kinds of
            plot. This tool is to be used with the generic TSP protocol.

File      : Information / Actions upon available providers.

-----------------------------------------------------------------------
*/


/*
 * System includes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/*
 * GDISP+ includes.
 */
#include "gdisp_kernel.h"
#include "gdisp_prototypes.h"


/*
 --------------------------------------------------------------------
                             STATIC ROUTINES
 --------------------------------------------------------------------
*/


/*
 * Function in order to sort provider symbols alphabetically.
 */
static gint
gdisp_sortProviderSymbolByName ( gconstpointer data1,
				 gconstpointer data2 )
{

  Symbol_T *symbol1 = (Symbol_T*)data1,
           *symbol2 = (Symbol_T*)data2;

  return (strcmp(symbol1->sInfo.name,symbol2->sInfo.name));

}


/*
 * Function in order to sort provider symbols according to their index.
 */
static gint
gdisp_sortProviderSymbolByIndex ( gconstpointer data1,
				  gconstpointer data2 )
{

  Symbol_T *symbol1 = (Symbol_T*)data1,
           *symbol2 = (Symbol_T*)data2;

  return (symbol1->sInfo.provider_global_index -
	  symbol2->sInfo.provider_global_index);

}


/*
 * Get provider status as a string.
 */
static gchar*
gdisp_providerStatusToString ( ThreadStatus_T status,
			       guint          *colorId )
{

  gchar *sStatus = (gchar*)NULL;

  switch (status) {

  case GD_THREAD_STOPPED :
    sStatus  = "STOPPED";
    *colorId = _WHITE_;
    break;

  case GD_THREAD_STARTING :
    sStatus = "STARTING...";
    *colorId = _ORANGE_;
    break;

  case GD_THREAD_WARNING :
    sStatus = "WARNING";
    *colorId = _ORANGE_;
    break;

  case GD_THREAD_REQUEST_SAMPLE_ERROR :
    sStatus = "REQUEST SAMPLE";
    *colorId = _RED_;
    break;

  case GD_THREAD_SAMPLE_INIT_ERROR :
    sStatus = "SAMPLE INIT";
    *colorId = _RED_;
    break;

  case GD_THREAD_SAMPLE_DESTROY_ERROR :
    sStatus = "SAMPLE DESTROY";
    *colorId = _RED_;
    break;

  case GD_THREAD_ERROR :
    sStatus = "THREAD ERROR";
    *colorId = _RED_;
    break;

  case GD_THREAD_RUNNING :
    sStatus = "RUNNING";
    *colorId = _GREEN_;
    break;

  default :
    sStatus = "UNKNOWN";
    *colorId = _GREY_;
    break;

  }

  return sStatus;

}


/*
 *  Graphically show the status of all providers.
 */
static void
gdisp_poolProviderThreadStatus ( Kernel_T *kernel )
{

  GList      *providerItem    = (GList*)NULL;
  Provider_T *provider        = (Provider_T*)NULL;
  Pixmap_T   *pixmap          = (Pixmap_T*)NULL;
  guint       providerLoad    = 0;
  guint       bgColorId       = _WHITE_;
  gchar       rowBuffer[128];


  /*
   * Loop over all available providers into the list.
   */
  providerItem = g_list_first(kernel->providerList);
  while (providerItem != (GList*)NULL) {

    provider = (Provider_T*)providerItem->data;

    if (gdisp_getProviderNumber(kernel) > 1) {

      pixmap = gdisp_getProviderIdPixmap(kernel,
					 provider->pCList,
					 provider->pIdentity);

      gtk_clist_set_pixtext(GTK_CLIST(provider->pCList),
			    0, /* status row    */
			    1, /* second column */
	       gdisp_providerStatusToString(provider->pSamplingThreadStatus,
					    &bgColorId),
			    5, /* spacing */
			    pixmap->pixmap,
			    pixmap->mask);

    }
    else {

      gtk_clist_set_text(GTK_CLIST(provider->pCList),
			 0 /* status row */,
			 1 /* information */,
	       gdisp_providerStatusToString(provider->pSamplingThreadStatus,
					    &bgColorId));

    }

    gtk_clist_set_background(GTK_CLIST(provider->pCList),
			     0 /* status row */,
			     &kernel->colors[bgColorId]);

    sprintf(rowBuffer,
	    "%d",
	    provider->pSampleList.TSP_sample_symbol_info_list_t_len);

    gtk_clist_set_text(GTK_CLIST(provider->pCList),
		       9 /* sample symbol row */,
		       1 /* information       */,
		       rowBuffer);

    /* ----------------------------------------------------------- */

    providerLoad    = provider->pLoad;
    provider->pLoad = 0; /* re-init for next cycle */

    if (providerLoad > provider->pMaxLoad) {

      /*
       * This may happend because GTK timers are not very precise...
       * So a 1 second period timer does not occur exactly every second.
       * So percentage may be higher than 100 %.
       */
      providerLoad = provider->pMaxLoad;

    }

    sprintf(rowBuffer,
	    "%d Bytes/s, %d %c",
	    providerLoad,
	    (guint)(100.0 * (gfloat)providerLoad / (gfloat)provider->pMaxLoad),
	    '%');

    gtk_clist_set_text(GTK_CLIST(provider->pCList),
		       10 /* provider load row */,
		       1  /* information       */,
		       rowBuffer);

    /* ----------------------------------------------------------- */

    providerItem = g_list_next(providerItem);

  } /* while (providerItem != (GList*)NULL) */

}


/*
 * Insert all providers into the vertical container box.
 */
static void
gdisp_insertAllProviders ( Kernel_T *kernel )
{

  GtkWidget  *frame            =  (GtkWidget*)NULL;
  GtkWidget  *hBox             =  (GtkWidget*)NULL;
  GtkWidget  *pixmapWidget     =  (GtkWidget*)NULL;
  Pixmap_T   *pixmap           =   (Pixmap_T*)NULL;

  GList      *providerItem     =      (GList*)NULL;
  Provider_T *provider         = (Provider_T*)NULL;

  gchar      *rowInfo  [  2];
  guint       rowNumber        = 0;
  gchar       rowBuffer[128];
  guint       bgColorId        = _WHITE_;


  /* ---------------- PER PROVIDER ------------- PER PROVIDER --------- */

  providerItem = g_list_first(kernel->providerList);
  while (providerItem != (GList*)NULL) {

    provider = (Provider_T*)providerItem->data;

    provider->pColor = gdisp_getProviderColor(kernel,
					      provider->pIdentity);


    /* ------------------------ FRAME WITH LABEL ------------------------ */

    /*
     * Create a Frame that will contain all provider information.
     * Align the label at the left of the frame.
     * Set the style of the frame.
     */
    sprintf(rowBuffer," URL : %s ",provider->pUrl->str);
    frame = gtk_frame_new(rowBuffer);

    gtk_frame_set_label_align(GTK_FRAME(frame),0.1,0.0);
    gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_ETCHED_IN);
    gtk_box_pack_start(GTK_BOX(kernel->widgets.providerVBox),
		       frame,
		       FALSE, /* expand  */
		       FALSE, /* fill    */
		       0);    /* padding */
    gtk_widget_show(frame);

    /* ------------------------ HORIZONTAL BOX  ------------------------ */

    /*
     * Create a horizontal packing box.
     */
    hBox = gtk_hbox_new(FALSE, /* homogeneous */
			5      /* spacing     */ );
    gtk_container_border_width(GTK_CONTAINER(hBox),10);
    gtk_container_add(GTK_CONTAINER(frame),hBox); 
    gtk_widget_show(hBox);


    /* ---------------------- PROVIDER LOGO ---------------------- */

    /*
     * Use GDK services to create provider Logo (XPM format).
     */
    pixmap = gdisp_getPixmapById(kernel,
				 GD_PIX_stubProvider,
				 kernel->widgets.dataBookWindow);

    pixmapWidget = gtk_pixmap_new(pixmap->pixmap,
				  pixmap->mask);

    gtk_box_pack_start(GTK_BOX(hBox),
		       pixmapWidget,
		       FALSE, /* expand  */
		       FALSE, /* fill    */
		       0);    /* padding */
    gtk_widget_show(pixmapWidget);


    /* -------------------------- CLIST --------------------------- */

    /*
     * A CList for containing all information.
     */
    provider->pCList = gtk_clist_new(2 /* columns */);

    gtk_clist_set_shadow_type(GTK_CLIST(provider->pCList),
			      GTK_SHADOW_ETCHED_IN);

    gtk_clist_set_button_actions(GTK_CLIST(provider->pCList),
				 0, /* left button */
				 GTK_BUTTON_IGNORED);

    gtk_clist_set_column_auto_resize(GTK_CLIST(provider->pCList),
				     0, /* first column */
				     TRUE);

    /* ------------------ LABELS WITH INFORMATION ------------------- */

    rowInfo[0] = "Status";
    rowInfo[1] = gdisp_providerStatusToString(provider->pSamplingThreadStatus,
					      &bgColorId);

    /* 'rowNumber' is 0 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    if (gdisp_getProviderNumber(kernel) > 1) {

      pixmap = gdisp_getProviderIdPixmap(kernel,
					 provider->pCList,
					 provider->pIdentity);

      gtk_clist_set_pixtext(GTK_CLIST(provider->pCList),
			    rowNumber,
			    1, /* second column */
			    rowInfo[1],
			    5, /* spacing */
			    pixmap->pixmap,
			    pixmap->mask);

    }

    /* do not remove */
    rowInfo[1] = rowBuffer;

    rowInfo[0] = "Version ID";
    sprintf(rowInfo[1],"V%d",provider->pVersionId);

    /* 'rowNumber' is 1 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Channel ID";
    sprintf(rowInfo[1],"%d",provider->pChannelId);

    /* 'rowNumber' is 2 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Timeout";
    sprintf(rowInfo[1],"%d ms",provider->pTimeOut);

    /* 'rowNumber' is 3 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Group Number";
    sprintf(rowInfo[1],"%d",provider->pGroupNumber);

    /* 'rowNumber' is 4 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Base Frequency";
    sprintf(rowInfo[1],"%3.0f Hz",provider->pBaseFrequency);

    /* 'rowNumber' is 5 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Maximum Period";
    sprintf(rowInfo[1],"%d ms",provider->pMaxPeriod);

    /* 'rowNumber' is 6 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Client Number";
    sprintf(rowInfo[1],
	    "%d (maximum %d)",
	    provider->pCurrentClientNumber,
	    provider->pMaxClientNumber);

    /* 'rowNumber' is 7 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Total Symbols";
    sprintf(rowInfo[1],"%d",provider->pSymbolNumber);

    /* 'rowNumber' is 8 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    rowInfo[0] = "Sampled Symbols";

    sprintf(rowInfo[1],
	    "%d",
	    provider->pSampleList.TSP_sample_symbol_info_list_t_len);

    /* 'rowNumber' is 9 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    /* ----------------------------------------------------------- */

    rowInfo[0] = "Detected Flow";
    sprintf(rowInfo[1],"0 Bytes/s, 0 %c",'%');

    /* 'rowNumber' is 10 */
    rowNumber  = gtk_clist_append(GTK_CLIST(provider->pCList),
				  rowInfo);

    /* FIXME : be careful with 'rowNumber' value */

    /* ----------------------------------------------------------- */

    gtk_box_pack_start(GTK_BOX(hBox),
		       provider->pCList,
		       TRUE, /* expand  */
		       TRUE, /* fill    */
		       0);   /* padding */

    gtk_widget_show(provider->pCList);


    /*
     * Next provider.
     */
    providerItem = g_list_next(providerItem);

  }

}


/*
 --------------------------------------------------------------------
                             PUBLIC ROUTINES
 --------------------------------------------------------------------
*/


/*
 * These two procedures deal with provider thread management.
 * In order to graphically show the status of all providers, a procedure
 * is registered in order to pool provider thread status.
 * The procedure is registered when the provider page is show in the databook.
 * The procedure is unregistered when other pages are shown in the databook.
 */
void
gdisp_providerTimer ( Kernel_T  *kernel,
		      gboolean   timerIsStarted )
{

#if defined(PROVIDER_DEBUG)

  fprintf(stdout,
	  "Provider Timer : %s\n",
	  timerIsStarted == TRUE ? "STARTED" : "STOPPED");
  fflush (stdout);

#endif

  switch (timerIsStarted) {

  case TRUE :

    /* refresh immediately */
    gdisp_poolProviderThreadStatus(kernel);

    (*kernel->registerAction)(kernel,
			      gdisp_poolProviderThreadStatus);

    break;

  case FALSE :

    /* stop refreshing thread status */
    (*kernel->unRegisterAction)(kernel,
				gdisp_poolProviderThreadStatus);

    break;

  }

}


/*
 * Create GDISP+ provider list.
 */
void
gdisp_createProviderList ( Kernel_T  *kernel,
			   GtkWidget *parent )
{

  GtkWidget *frame          = (GtkWidget*)NULL;
  GtkWidget *scrolledWindow = (GtkWidget*)NULL;


  /* ------------------------ FRAME WITH LABEL ------------------------ */

  /*
   * Create a Frame that will contain a scrolled window for providers.
   * Align the label at the left of the frame.
   * Set the style of the frame.
   */
  frame = gtk_frame_new(" Available Providers ");
  gtk_frame_set_label_align(GTK_FRAME(frame),0.1,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_ETCHED_IN);

  gtk_container_add(GTK_CONTAINER(parent),frame);
  gtk_widget_show(frame);

  /* ----------- SCROLLED WINDOW FOR THE LIST OF PROVIDERS  ----------- */

  /*
   * This is the scrolled window to put the List widget inside.
   */
  scrolledWindow = gtk_scrolled_window_new(NULL /* H Adjustment */,
					   NULL /* V Adjustment */);
  gtk_container_border_width(GTK_CONTAINER(scrolledWindow),5);
  gtk_container_add(GTK_CONTAINER(frame),scrolledWindow); 
  gtk_widget_show(scrolledWindow);

  /* ----------- VERTICAL BOX FOR HANDLING ALL PROVIDERS  ----------- */

  /*
   * We need a vertical packing box for managing all providers.
   */
  kernel->widgets.providerVBox = gtk_vbox_new(FALSE, /* homogeneous */
		                              5      /* spacing     */ );
  gtk_container_border_width(GTK_CONTAINER(kernel->widgets.providerVBox),10);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledWindow),
					kernel->widgets.providerVBox);
  gtk_widget_show(kernel->widgets.providerVBox);

  /* ---------------------- INSERT ALL PROVIDERS ---------------------- */

  gdisp_insertAllProviders(kernel);

}


/*
 * Destroy GDISP+ provider list.
 */
void
gdisp_destroyProviderList ( Kernel_T *kernel )
{

  /*
   * Nothing by now.
   */

}


/*
 * Refresh provider graphic list.
 */
void
gdisp_refreshProviderList ( Kernel_T *kernel )
{

  GList *childrenList = (GList*)NULL;
  GList *frameList    = (GList*)NULL;

  /* -------------------- REMOVE PREVIOUS PROVIDERS ------------------- */

  childrenList =
    gtk_container_children(GTK_CONTAINER(kernel->widgets.providerVBox));

  frameList = g_list_first(childrenList);

  while (frameList != (GList*)NULL) {

    gtk_widget_destroy((GtkWidget*)frameList->data);
    frameList = g_list_next(frameList);

  }

  /* ---------------------- INSERT ALL PROVIDERS ---------------------- */

  gdisp_insertAllProviders(kernel);

}


/*
 * Sort provider symbols by name or index.
 */
void
gdisp_sortProviderSymbols ( Kernel_T        *kernel,
			    SortingMethod_T  sortingMethod )
{

  GList      *providerItem = (GList*)NULL;
  Provider_T *provider     = (Provider_T*)NULL;

  /*
   * Loop upon all providers.
   */
  providerItem = g_list_first(kernel->providerList);
  while (providerItem != (GList*)NULL) {

    provider = (Provider_T*)providerItem->data;

    switch (sortingMethod) {

    case GD_SORT_BY_NAME :
      qsort((void*)provider->pSymbolList,
	    provider->pSymbolNumber,
	    sizeof(Symbol_T),
	    gdisp_sortProviderSymbolByName);
      break;

    case GD_SORT_BY_INDEX :
      qsort((void*)provider->pSymbolList,
	    provider->pSymbolNumber,
	    sizeof(Symbol_T),
	    gdisp_sortProviderSymbolByIndex);
      break;

    case GD_SORT_BY_PROVIDER     :
    case GD_SORT_BY_TYPE         :
    case GD_SORT_BY_DIM          :
    case GD_SORT_BY_EXTINFO      :
    default                      :
      /* not used here */
      break;

    }

    providerItem = g_list_next(providerItem);

  } /* while */

}
