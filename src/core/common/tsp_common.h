/*!  \file 

$Id: tsp_common.h,v 1.2 2005-10-09 23:01:23 erk Exp $

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

Project    : TSP
Maintainer : tsp@astrium.eads.net
Component  : Common

-----------------------------------------------------------------------

Purpose   : Main interface for the producer module

-----------------------------------------------------------------------
 */

#ifndef _TSP_COMMON_H
#define _TSP_COMMON_H

#include "tsp_prjcfg.h"
#include "tsp_datastruct.h"

typedef enum TSP_request_type {
  E_TSP_REQUEST_INVALID = 0,
  E_TSP_REQUEST_GENERIC,
  E_TSP_REQUEST_OPEN,
  E_TSP_REQUEST_CLOSE,
  E_TSP_REQUEST_INFORMATION,
  E_TSP_REQUEST_FILTERED_INFORMATION,
  E_TSP_REQUEST_FEATURE,
  E_TSP_REQUEST_SAMPLE,
  E_TSP_REQUEST_SAMPLE_INIT,
  E_TSP_REQUEST_SAMPLE_DESTROY,
  E_TSP_REQUEST_ASYNC_SAMPLE_WRITE,
  E_TSP_REQUEST_ASYNC_SAMPLE_READ,
  E_TSP_REQUEST_LAST
} TSP_request_type_t;

#ifdef TSP_COMMON_C
const char* tsp_reqname_tab[] = {"tsp_request_invalid",
				 "tsp_request_generic",
				 "tsp_request_open",
				 "tsp_request_close",
				 "tsp_request_information",
				 "tsp_request_filtered_information",
				 "tsp_request_feature",
				 "tsp_request_sample",
				 "tsp_request_sample_init",
				 "tsp_request_sample_destroy",
				 "tsp_request_async_sample_write",
				 "tsp_request_async_sample_read",
				 "tsp_request_last"
};

const char* tsp_reqhelp_tab[] = {"invalid tsp request",
				 "tsp_request_generic",
				 "tsp_request_open",
				 "tsp_request_close",
				 "tsp_request_information",
				 "tsp_request_filtered_information",
				 "tsp_request_feature",
				 "tsp_request_sample",
				 "tsp_request_sample_init",
				 "tsp_request_sample_destroy",
				 "tsp_async_sample_write",
				 "tsp_async_sample_read",
				 "TSP LAST REQUEST"
};
#else
extern const char* tsp_reqname_tab[];
extern const char* tsp_reqhelp_tab[];
#endif

typedef struct TSP_request {
  int                 version_id;
  TSP_request_type_t    req_type;
  void*                 req_data;
} TSP_request_t;


/**
 * Initialize a TSP request.
 * @param req 
 * @param req_type
 */
void TSP_request_create(TSP_request_t* req, TSP_request_type_t req_type);

void
TSP_common_sample_symbol_info_list_copy(TSP_sample_symbol_info_list_t* dest_symbols, 
					TSP_sample_symbol_info_list_t  src_symbols);
void
TSP_common_sample_symbol_copy(TSP_sample_symbol_info_t* dest_symbol, 
			      TSP_sample_symbol_info_t src_symbol);
#endif /* _TSP_COMMON_H */