/*

$Id: glue_res.c,v 1.15 2008-02-05 18:54:12 rhdv Exp $

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

Purpose   : Implementation for the glue_server

-----------------------------------------------------------------------
*/
#include <string.h>

#include <tsp_sys_headers.h>
#include <tsp_glu.h>
#include <tsp_ringbuf.h>
#include <tsp_time.h>
#include <tsp_datapool.h>
#include <tsp_common.h>


#define _LIBUTIL_REENTRANT 1
#include "libUTIL.h"

#define GLU_STREAM_INIT_USAGE "TSP RES Reader usage : filename[.res]  [-eof N]\n\t\twait N seconds after EOF to check file increment, default = 0 (NO WAIT)\n"
#define GLU_RES_FILE_ARG_NUMBER 1
#define GLU_RES_WAIT_EOF       	3

static int _wait_eof=0;
static int _started = FALSE;
static GLU_handle_t* res_GLU = NULL;

struct GLU_state_t
{
  void* res_values;
  int time_stamp;
  int nbvar;
  int use_dbl;
  double freq;
  d_rhandle h_res;
  TSP_sample_symbol_info_list_t* ssiList;
};

typedef struct GLU_state_t GLU_state_t;

GLU_state_t glu_handler;


void RES_GLU_loop()
{
  GLU_state_t* obj = &glu_handler;
  glu_item_t  item;
  int i;
  GLU_get_state_t state;
  int run = _wait_eof+1;
  double dbl_value;
  

  item.raw_value = &dbl_value;

  while(!_started)
    sleep(1);

  while(run)
    {
      if(d_read_r(obj->h_res, obj->res_values) == EOF)
	{
	  STRACE_DEBUG("EOF for time=%d, val[0]=%g", obj->time_stamp, *((double*)item.raw_value));
	  if(!_wait_eof)
	    {
	      state = GLU_GET_EOF;
	      run = FALSE;
	    }
	  else
	    {
	      state = GLU_GET_NO_ITEM;
	      sleep(1); /* wait 1 s for update of file */
	      run--;
	    }
	}
      else
	{
	  /* got a new RES line : reset time-out counter and increment time_stamp */
	  run = _wait_eof+1;
	  obj->time_stamp++;
	  state = GLU_GET_NEW_ITEM;
	 
	  /* push all data of line */
	  for(i=0; i<obj->nbvar; i++)
	    {
	      item.provider_global_index = i;
	      *((double*)item.raw_value) = obj->use_dbl ?
		((double*)(obj->res_values)) [i] :
	        ((float*) (obj->res_values)) [i];
	      item.size                  = tsp_type_size[obj->ssiList->TSP_sample_symbol_info_list_t_val[i].type];
	      
	      TSP_datapool_push_next_item(res_GLU->datapool,&item);

	      if(i==0)
		STRACE_INFO("New record : time=%d, val[0]=%g", obj->time_stamp, *((double*)item.raw_value));
	    }
      
	}

      /* commit ... */
      TSP_datapool_push_commit(res_GLU->datapool,obj->time_stamp, state);     
    }

  /* close .res file */
  d_rclos_r(obj->h_res);
}

int RES_GLU_start(GLU_handle_t* this)
{
  _started = TRUE;
  return TRUE;
}


int RES_GLU_init(GLU_handle_t* this, int fallback_argc, char* fallback_argv[])
{

  int ret = TRUE;
  GLU_state_t* obj = &glu_handler;
  int i;
  double t1, t2;
  int nbrec;  
  char namev[RES_NAME_LEN];
  char descv[RES_DESC_LEN];

  /* is there a fallback stream ? */
  if(fallback_argc>1 && fallback_argv)
    {
      /* Yes, we must test it. We are expectig one arg */
      char* res_file = fallback_argv[GLU_RES_FILE_ARG_NUMBER];
      /* FIXME : quand la fonction renvera un code d'erreur, le verifier ! */
      obj->time_stamp = -1;

      STRACE_INFO("stream_init = '%s'", res_file);
      obj->h_res = d_ropen_r(res_file, &(obj->use_dbl));
	 
      if( obj->h_res )
	{
	  obj->nbvar = d_rval_r(obj->h_res, 'v');
	  nbrec = d_rval_r(obj->h_res, 'r');

	  STRACE_INFO("Total number of records = %d", nbrec);
	  STRACE_INFO("Total number of variables = %d", obj->nbvar);
	  STRACE_INFO("Data type = %s", obj->use_dbl ? "DOUBLE" : "FLOAT");

	  obj->ssiList = TSP_SSIList_new(obj->nbvar+1);
	  assert(obj->ssiList);

	  for (i=0; i<obj->nbvar; i++) {      
	    d_rnam_r(obj->h_res, namev, descv, i);
	    TSP_SSI_initialize(&(obj->ssiList->TSP_sample_symbol_info_list_t_val[i]),
			       namev,
			       i, /* PGI */
			       0, /* group rank idx */
			       0, /* group rank */
			       /* obj->use_dbl ? TSP_TYPE_DOUBLE : TSP_TYPE_FLOAT, */
			       TSP_TYPE_DOUBLE, /* force DOUBLE for compatibility reason */
			       1, /* dimension */
			       0, /* offset */
			       1, /* nelem */
			       1, /* period */
			       0); /* phase */
	  }
  
	  obj->res_values = obj->use_dbl ?
	    calloc((obj->nbvar+1),sizeof(double))
	    : calloc((obj->nbvar+1),sizeof(float));
	  TSP_CHECK_ALLOC(obj->res_values, FALSE);
	      
	  /* Try to compute res file frequency */
	  d_read_r(obj->h_res, obj->res_values);
	  t1 = obj->use_dbl ? ((double*)(obj->res_values)) [0] : ((float*)(obj->res_values)) [0];
	  d_read_r(obj->h_res, obj->res_values);
	  t2 = obj->use_dbl ? ((double*)(obj->res_values)) [0] : ((float*)(obj->res_values)) [0];
	  if (t2!=t1)
	    obj->freq = 1/(t2-t1);
	  else
	    obj->freq = 0;
	  d_restart_r(obj->h_res); /* ask to restart from begining */

	}
      else
	{
	  STRACE_ERROR("Function d_rval_r failed for file : '%s'", res_file);
	}

       if(fallback_argc > GLU_RES_WAIT_EOF)
	{
	  _wait_eof = atoi(fallback_argv[GLU_RES_WAIT_EOF]);
	}
    }
  else
    {
      STRACE_ERROR(GLU_STREAM_INIT_USAGE);
      ret = FALSE;
    }

  return ret;
}


int  RES_GLU_get_sample_symbol_info_list(GLU_handle_t* h_glu,TSP_sample_symbol_info_list_t* symbol_list)
{
  GLU_state_t* obj = &glu_handler;

  symbol_list->TSP_sample_symbol_info_list_t_len = obj->nbvar;
  symbol_list->TSP_sample_symbol_info_list_t_val = obj->ssiList->TSP_sample_symbol_info_list_t_val;
	    
  return TRUE;
}


double RES_GLU_get_base_frequency(GLU_handle_t* this)
{
  GLU_state_t* obj = &glu_handler;
  /* Server is passive, frequency is computed in GLU_init */
  return obj->freq;
}

GLU_handle_t* 
RES_GLU_get_instance(GLU_handle_t* this,
			 int custom_argc,
			 char* custom_argv[],
			 char** error_info) {
  

    return this;

} /* end of GLU_get_instance_default */

GLU_handle_t* GLU_resreader_create() {
  
  /* create a default GLU */
  GLU_handle_create(&res_GLU,"ResServer",GLU_SERVER_TYPE_PASSIVE,1.0);
  
  res_GLU->initialize         = &RES_GLU_init;
  /* res_GLU->run                = &RES_GLU_thread;  */
  res_GLU->get_ssi_list       = &RES_GLU_get_sample_symbol_info_list;
  /* override default method */
  res_GLU->get_base_frequency = &RES_GLU_get_base_frequency;
  res_GLU->start              = &RES_GLU_start;
  /* FIXME seems that resreader should have been multi-instance but has never been? */
  res_GLU->get_instance       = &RES_GLU_get_instance;

  return res_GLU;
}
