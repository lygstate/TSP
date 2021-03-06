/*

$Id: server_main.c,v 1.4 2006-04-24 21:05:34 erk Exp $

-----------------------------------------------------------------------

TSP Library - core components for a generic Transport Sampling Protocol.

Copyright (c) 2002 Yves DUFRENNE, Stephane GALLES, Eric NOULARD and Robert PAGNOT 

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
Component : Provider

-----------------------------------------------------------------------

Purpose   : Implementation for the glue_server, for stub test
	    Allow the output of a datapool of 1000 symbols, 
	    cyclically generated at 100Hz

-----------------------------------------------------------------------
*/
#include <stdio.h>
#include "tsp_provider_init.h"
#include "tsp_sys_headers.h"

GLU_handle_t* GLU_stub_create();
 
int TSP_launcher () 
{
int argc = 1 ;
char **argv;
char *tableau[] ={"TSP_launcher"} ;
	argv=tableau ;
  printf ("#===================================================================#\n");
  printf ("# Launching <StubbedServer> for generation of 1000 Symbols at 100Hz    #\n");
  printf ("#===================================================================#\n");

  GLU_handle_t* GLU_stub = GLU_stub_create();
  /* Init server */
  if (TSP_STATUS_OK==TSP_provider_init(GLU_stub,&argc, &argv)) {
    if (TSP_STATUS_OK!=TSP_provider_run(TSP_ASYNC_REQUEST_SIMPLE | TSP_ASYNC_REQUEST_NON_BLOCKING)) {
      return (int)-1;
    }
  }
  return (int)0 ;
}
   
void smallTester (int *arg) ;
void smallTester (int *arg) 
{
logMsg ("My id is : %0X arg = %0X\n",pthread_self(),*arg,0,0,0,0) ;

}

void miniLaunch (void) ;
void miniLaunch () 
{
pthread_t newThread ;
int err ;
int toto =taskIdSelf();
logMsg ("My id is : %0X\n",toto,0,0,0,0,0) ;
err = pthread_create (&newThread,NULL,(void *)smallTester,&toto) ;
logMsg ("My son is %0X\n",newThread,0,0,0,0,0) ;

}
