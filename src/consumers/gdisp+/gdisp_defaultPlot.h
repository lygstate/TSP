/*

$Id: gdisp_defaultPlot.h,v 1.4 2006-08-05 20:50:30 esteban Exp $

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
Maintainer: tsp@astrium-space.com
Component : Graphic Tool

-----------------------------------------------------------------------

Purpose   : Graphic Tool based on GTK+ that provide several kinds of
            plot. This tool is to be used with the generic TSP protocol.

File      : Definition of 'default plot' private structures.

-----------------------------------------------------------------------
*/

#ifndef __DEFAULT_PLOT_H__
#define __DEFAULT_PLOT_H__

/*
 * Private structure of a 'default plot'.
 */
typedef struct DefaultPlot_T_ {

  /*
   * Attributes.
   */
  gboolean   dpHasFocus;

  /*
   * Graphic widget.
   */
  GtkWidget *dpArea;
  guint      dpAreaWidth;
  guint      dpAreaHeight;
  GdkGC     *dpGContext;

  /*
   * Parent widget.
   */
  GtkWidget *dpParent;

} DefaultPlot_T;


#endif /* __DEFAULT_PLOT_H__ */
