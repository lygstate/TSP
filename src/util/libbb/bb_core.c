/*

$Header: /home/def/zae/tsp/tsp/src/util/libbb/bb_core.c,v 1.59 2008-07-24 13:27:33 jaggy Exp $

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
Maintainer : tsp@astrium-space.com
Component : Consumer

-----------------------------------------------------------------------

Purpose   : Blackboard Idiom implementation

-----------------------------------------------------------------------
 */

#ifndef __KERNEL__
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#if !defined(__rtems__)
#  include <dlfcn.h>
#  include <sys/mman.h>
#endif
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT (void *)0
#endif

#include <tsp_abs_types.h>
#include <tsp_abs_types.h>
#include <tsp_sys_headers.h>
#include <tsp_common_trace.h>

#endif /* __KERNEL__ */

#include "bb_alias.h"
#include "bb_utils.h"
#include "bb_core.h"
#include "bb_local.h"

/**
 * Convert type to string for display use.
 */
#ifndef __KERNEL__
static const char* E_BB_2STRING[] = {"DiscoverType",
				     "double",
				     "float",
				     "int8_t",
				     "int16_t",
				     "int32_t",
				     "int64_t",
				     "uint8_t",
				     "uint16_t",
				     "uint32_t",
				     "uint64_t",
				     "char",
				     "uchar",
				     "UserType",
				     "NotAType end"};
#endif

static const size_t E_BB_TYPE_SIZE[] = {0,
/* #ifndef __KERNEL__ */
					sizeof(double),
					sizeof(float),
/* #else */
/*					0, */
/*					0, */
/* #endif */
					sizeof(int8_t),
					sizeof(int16_t),
					sizeof(int32_t),
					sizeof(int64_t),
					sizeof(uint8_t),
					sizeof(uint16_t),
					sizeof(uint32_t),
					sizeof(uint64_t),
					sizeof(char),
					sizeof(unsigned char),
					0,
					0};

/* In case we're compiling kernel code, there's really no need
   for the sysv or posix code */
#ifndef __KERNEL__

extern struct bb_operations sysv_bb_ops;

#if defined(USE_POSIX_BB)
extern struct bb_operations posix_bb_ops;
#endif

#endif /*! __KERNEL__*/

#if	   (defined(linux) || defined(__linux))				\
	&& (defined (CONNECTOR_AVAILABLE) || defined (__KERNEL__))
extern struct bb_operations k_bb_ops;
#endif /* linux */

/* Note: This is thigtly related to enum bb_type in bb_core.h,
   if you ever modify this, you'll probably need to modify
   enum bb_type */
static struct bb_operations *ops[] = {
#if !defined(__rtems__) && !defined (__KERNEL__)
	&sysv_bb_ops
#else
	NULL
#endif
#if	   (defined(linux) || defined(__linux))				\
	&& (defined (CONNECTOR_AVAILABLE) || defined (__KERNEL__))
	,&k_bb_ops
#else
	,NULL
#endif /* linux */
#if defined(USE_POSIX_BB) && !defined (__KERNEL__)
	,&posix_bb_ops
#else
	,NULL
#endif
};

static enum bb_type bb_type(const char *name)
{
#ifdef __KERNEL__
	return BB_KERNEL;
#endif /* __KERNEL__ */

	if (!strncmp(name, "/dev/", 5))
		return BB_KERNEL;
	if (!strncmp(name, "p:/", 3)) {
		return BB_POSIX;
	}
	return BB_SYSV;
}

size_t sizeof_bb_type(E_BB_TYPE_T bb_type) {
  size_t retval = 0;
  if ((bb_type >= 1) && (bb_type < E_BB_USER)) {
    retval = E_BB_TYPE_SIZE[bb_type];
  }
  return retval;
}

E_BB_TYPE_T 
bb_type_string2bb_type(const char* bb_type_string) {
  E_BB_TYPE_T retval = 0;
#ifndef __KERNEL__
  if (!strncasecmp("double",bb_type_string,strlen("double"))) {
    retval = E_BB_DOUBLE;
  } else
  if (!strncasecmp("float",bb_type_string,strlen("float"))) {
    retval = E_BB_FLOAT;
  } else
#endif
  if (!strncasecmp("int8",bb_type_string,strlen("int8"))) {
    retval = E_BB_INT8;
  } else
  if (!strncasecmp("int16",bb_type_string,strlen("int16"))) {
    retval = E_BB_INT16;
  } else
  if (!strncasecmp("int32",bb_type_string,strlen("int32"))) {
    retval = E_BB_INT32;
  } else
  if (!strncasecmp("int64",bb_type_string,strlen("int64"))) {
    retval = E_BB_INT64;
  } else
  if (!strncasecmp("uint8",bb_type_string,strlen("uint8"))) {
    retval = E_BB_UINT8;
  } else
  if (!strncasecmp("uint16",bb_type_string,strlen("uint16"))) {
    retval = E_BB_UINT16;
  } else
  if (!strncasecmp("uint32",bb_type_string,strlen("uint32"))) {
    retval = E_BB_UINT32;
  } else
  if (!strncasecmp("uint64",bb_type_string,strlen("uint64"))) {
    retval = E_BB_UINT64;
  } else
  if (!strncasecmp("char",bb_type_string,strlen("char"))) {
    retval = E_BB_CHAR;
  } else
  if (!strncasecmp("uchar",bb_type_string,strlen("uchar"))) {
    retval = E_BB_UCHAR;
  }
  return retval;
}

#ifdef __KERNEL__
static inline char *__strdup (const char *s1)
{
	char *p;
	p = (char *)kmalloc((strlen(s1)+1)*sizeof(char), GFP_KERNEL);
	if (p != NULL)
		strcpy (p,s1);
	return p;
}
#endif
static char *bb_get_varname_default(const S_BB_DATADESC_T *dd)
{
#ifdef __KERNEL__
  return __strdup(dd->__name);
#else
  return strdup(dd->__name);
#endif
}

static int32_t bb_varname_max_len_default(void)
{
  return VARNAME_MAX_SIZE;
}

static int32_t bb_set_varname_default(S_BB_DATADESC_T *dd, const char *key)
{
  strncpy(dd->__name, key, VARNAME_MAX_SIZE);
  return BB_OK;
}

bb_get_varname_fn bb_get_varname = bb_get_varname_default;
bb_set_varname_fn bb_set_varname = bb_set_varname_default;
bb_varname_max_len_fn bb_varname_max_len = bb_varname_max_len_default;

#if !defined (__KERNEL__)  && !defined (__rtems__)
static int32_t
bb_varname_init(S_BB_T *bb)
{
  bb_varname_init_fn init;
  bb_get_varname_fn getter;
  bb_set_varname_fn setter;
  bb_varname_max_len_fn max_len;
  char getter_name[256];
  char setter_name[256];
  char init_name[256];
  char max_len_name[256];
  S_BB_PRIV_T *priv;
  void *handle = RTLD_DEFAULT;

  priv = bb_get_priv(bb);
  if (priv->varname_lib[0] == '\0') {
    return BB_OK;
  }

  sprintf(getter_name, "bb_get_varname_%s", priv->varname_lib);
  sprintf(setter_name, "bb_set_varname_%s", priv->varname_lib);
  sprintf(init_name, "bb_varname_init_%s", priv->varname_lib);
  sprintf(max_len_name, "bb_varname_max_len_%s", priv->varname_lib);

  getter = dlsym(handle, getter_name);
  if (!getter) {
    char libname[FILENAME_MAX];

    sprintf(libname, "lib%s.so", priv->varname_lib);
    /* try to open a library */
    handle = dlopen(libname, RTLD_NOW);
    if (!handle) {
      bb_logMsg(BB_LOG_WARNING, "cound not find lib %s\n", libname);
      return BB_NOK;
    }
    getter = dlsym(handle, getter_name);
    if (!getter) {
      bb_logMsg(BB_LOG_WARNING, "cound not find symbol %s\n", getter_name);
      return BB_NOK;
    }
  }
  setter = dlsym(handle, setter_name);
  if (!setter) {
    bb_logMsg(BB_LOG_WARNING, "cound not find symbol %s\n", setter_name);
    return BB_NOK;
  }

  init = dlsym(handle, init_name);

  if (init) {
    if (init(bb) != BB_OK)
      return BB_NOK;
  }

  max_len = dlsym(handle, max_len_name);
  if (max_len)
    bb_varname_max_len = max_len;

  bb_set_varname = setter;
  bb_get_varname = getter;

  return BB_OK;
}
#else
static int32_t
bb_varname_init(S_BB_T *bb)
{
	return BB_OK;
}
#endif


int32_t
bb_ctl(S_BB_T *bb, unsigned int request, ...)
{
  va_list ap;
  int ret=BB_OK;
  char *name;
  S_BB_PRIV_T *priv;

  va_start(ap, request);
  switch (request) {
    /* We've been passed a function pointer */
    case BB_CTL_SET_NAME_SETTER_PTR:
      bb_set_varname = va_arg(ap, typeof(bb_set_varname));
      break;
      /* We've been passed a function pointer */
    case BB_CTL_SET_NAME_GETTER_PTR:
      bb_get_varname = va_arg(ap, typeof(bb_get_varname));
      break;
    case BB_CTL_GET_NAMING_FROM_BB:
      ret=bb_varname_init(bb);
      break;
    case BB_CTL_SET_NAMING_IN_BB:
      name = va_arg(ap, typeof(name));
      priv = bb_get_priv(bb);
      strncpy(priv->varname_lib, name, VARNAME_LIB_LENGTH);
      ret=bb_varname_init(bb);
      break;
    default:
      ret=BB_NOK;
      break;
  }
  va_end(ap);

  return ret;
}

int32_t
bb_check_version(volatile S_BB_T* bb) {
  int32_t retval = 0;
/*   union { */
/*     uint16_t lower; */
/*     uint16_t upper; */
/*   } version_using, version_access; */

  assert(bb);
  retval = BB_VERSION_ID - bb->bb_version_id;
  /* From now on we're nice and backwards compatible */
  if (BB_VERSION_ID >= 0x2000 && bb->bb_version_id >= 0x2000) {
    return 0; 
  }
/*   if (retval != 0) { */
/*       bb_logMsg(BB_LOG_WARNING, "BlackBoard::bb_check_version", */
/* 		"BB version mismatch using <0x%08X> for accessing <0x%08X>\n", */
/* 		BB_VERSION_ID,bb->bb_version_id); */
/*   } */
  return retval;
} /* end of bb_check_version */

int32_t 
bb_size(const int32_t n_data, const int32_t data_size) {
  /* The SHM segment is sized to:
   *  BB structure size +
   *  data descriptor array size +
   *  data zone size
   */
  int32_t size;

  size = (sizeof(S_BB_T) + 
    sizeof(S_BB_DATADESC_T)*n_data +
    sizeof(char)*data_size);

  size += sizeof(S_BB_PRIV_T);
  return size;
} /* end of bb_size */

S_BB_PRIV_T *
bb_get_priv(volatile S_BB_T* bb)
{
  return (S_BB_PRIV_T *)((unsigned long)bb + (unsigned long)(bb_size(bb->max_data_desc_size, bb->max_data_size) - sizeof(S_BB_PRIV_T)));
}

int32_t
bb_find(volatile S_BB_T* bb, const char* var_name) {
  int32_t retval;
  int32_t i;

  retval = -1;
  assert(bb);

  for (i=0; i< bb->n_data;++i) {
    char * n = bb_get_varname(&bb_data_desc(bb)[i]);
    if (!strcmp(var_name,n)) {
      retval = i;
      free(n);
      break;
    }
    free(n);
  } /* end for */

  return retval;
} /* end of  bb_find */

S_BB_DATADESC_T*
bb_data_desc(volatile S_BB_T* bb) {
  S_BB_DATADESC_T* retval;

  retval = NULL;
  assert(bb);
  retval = (S_BB_DATADESC_T*) ((char*)(bb) + bb->data_desc_offset);

  return retval;
} /* end of bb_data_desc */

void*
bb_data(volatile S_BB_T* bb) {
  void* retval;

  retval = NULL;
  assert(bb);
  retval = (char*)(bb) + bb->data_offset;

  return retval;
} /* end of bb_data */

#ifndef __KERNEL__
double
bb_double_of(void* value, E_BB_TYPE_T bbtype) {

  double retval;

  retval = 0.0;

  switch (bbtype) {
  case E_BB_DOUBLE: 
    retval = *((double*)value);
    break;
  case E_BB_FLOAT:
    retval = *((float*)value);
    break;
  case E_BB_INT8:
    retval = *((int8_t*)value);
    break; 
  case E_BB_INT16:
    retval = *((int16_t*)value);
    break; 
  case E_BB_INT32:
    retval = *((int32_t*)value);
    break; 
  case E_BB_INT64:
    retval = *((int64_t*)value);
    break; 
  case E_BB_UINT8:
    retval = *((uint8_t*)value);
    break;  
  case E_BB_UINT16:
    retval = *((uint16_t*)value);
    break;
  case E_BB_UINT32:
    retval = *((uint32_t*)value);
    break;	
  case E_BB_UINT64:
    retval = *((uint64_t*)value);
    break;	
  case E_BB_CHAR:
  case E_BB_UCHAR:
  case E_BB_USER:
    /* FIXME could not convert set 0.0 */
    retval = 0.0;
    break; 
  default:
    retval = 0.0;
    break;
  }

  return retval;
} /* end of bb_double_of */
#endif

int32_t 
bb_data_initialise(volatile S_BB_T* bb, S_BB_DATADESC_T* data_desc,void* default_value) {
  
  int32_t retval;  
  char* data;
  int32_t i;
  int32_t idxstack[1];
  int32_t idxstack_len = 1;
  
  idxstack[0] = 0;
  
  assert(data_desc);
  /* on recupere l'adresse de la donnee dans le BB */
  /* FIXME a modifier peut etre */
  //data = (char*)bb_data(bb) + data_desc.data_offset;
  data = bb_item_offset(bb,data_desc,idxstack, idxstack_len);
  
  
  retval = BB_OK;
  for (i=0; i< data_desc->dimension; ++i) {
    switch (data_desc->type) {
#ifndef __KERNEL__
    case E_BB_DOUBLE: 
      ((double*) data)[i] = default_value ? *((double *) default_value) : 0.0;
      break;
    case E_BB_FLOAT:
      ((float*) data)[i] = default_value ? *((float *) default_value) : 0.0;
      break;
#endif
    case E_BB_INT8:
      ((int8_t*) data)[i] = default_value ? *((int8_t *) default_value) : 0;
      break;
    case E_BB_INT16:
      ((int16_t*) data)[i] = default_value ? *((short *) default_value) : 0;
      break; 
    case E_BB_INT32:
      ((int32_t*) data)[i] = default_value ? *((int *) default_value) : 0;
      break; 
    case E_BB_INT64:
      ((int64_t*) data)[i] = default_value ? *((long *) default_value) : 0;
      break;       
    case E_BB_UINT8:
      ((uint8_t*) data)[i] = default_value ? *((uint8_t *) default_value) : 0;
      break;
    case E_BB_UINT16:
      ((uint16_t*) data)[i] = default_value ? *((unsigned short *) default_value) : 0;
      break;
    case E_BB_UINT32:
      ((uint32_t*) data)[i] = default_value ? *((unsigned int *) default_value) : 0;
      break;	
    case E_BB_UINT64:
      ((uint64_t*) data)[i] = default_value ? *((unsigned long *) default_value) : 0;
      break;	
    case E_BB_CHAR:
      ((char *) data)[i] = default_value ? *((char *) default_value) : '\0';
      break;
    case E_BB_USER:
      if (NULL == default_value) {
				memset(data + (data_desc->type_size)*i,0,data_desc->type_size);
      } else {
				memcpy(data + (data_desc->type_size)*i,default_value,data_desc->type_size);
      }
      break; 
    default:
      retval = BB_NOK;
      break;
    }
  } 
  
  return retval;
} /* end of bb_data_initialise */

int32_t
bb_value_direct_rawwrite(void* data, S_BB_DATADESC_T data_desc, void* value) {  
  memcpy(data,value,data_desc.type_size);
  return BB_OK;
}

#ifndef __KERNEL__
int32_t
bb_value_direct_write(void* data, S_BB_DATADESC_T data_desc, const char* value, int hexval) {

  int retval;

  retval = BB_OK;

  switch (data_desc.type) {
  case E_BB_DOUBLE: 
    ((double *)data)[0] = atof(value);
    break;
  case E_BB_FLOAT:
    ((float *)data)[0] = atof(value);
    break;
  case E_BB_INT8:
    ((int8_t*)data)[0] = strtol(value,(char **)NULL,hexval ? 16 : 10);
    break; 
  case E_BB_INT16:
    ((int16_t*)data)[0] = strtol(value,(char **)NULL,hexval ? 16 : 10);
    break; 
  case E_BB_INT32:
    ((int32_t*)data)[0] = strtol(value,(char **)NULL,hexval ? 16 : 10);
    break; 
  case E_BB_INT64:
    ((int64_t*)data)[0] = strtoll(value,(char **)NULL,hexval ? 16 : 10);
    break; 
  case E_BB_UINT8:
    ((uint8_t*)data)[0] = strtol(value,(char **)NULL,hexval ? 16 : 10);
    break; 
  case E_BB_UINT16:
    ((uint16_t*)data)[0] = strtoul(value,(char **)NULL,hexval ? 16 : 10);
    break;
  case E_BB_UINT32:
    ((uint32_t*)data)[0] = strtoul(value,(char **)NULL,hexval ? 16 : 10);
    break;	
  case E_BB_UINT64:
    ((uint64_t*)data)[0] = strtoull(value,(char **)NULL,hexval ? 16 : 10);
    break;	
  case E_BB_CHAR:
    memcpy(&((char*)data)[0],value,sizeof(char));
    break;
  case E_BB_UCHAR:
    memcpy(&((unsigned char*)data)[0],value,sizeof(unsigned char));
    break; 
  case E_BB_USER:
    
    retval = bb_utils_convert_string2hexbuf(hexval ? value+2 : value,
					    &((unsigned char*)data)[0*data_desc.type_size],
					    data_desc.type_size, 
					    hexval);
    retval = BB_NOK;
    break; 
  default:
    retval = BB_NOK;
    break;
  }
  return retval;
} /* bb_value_direct_write */
#endif /* !__KERNEL__ */

int32_t
bb_value_write(volatile S_BB_T* bb, 
	       S_BB_DATADESC_T data_desc, 
	       const char* value, 
	       int32_t* idxstack, int32_t idxstack_len) {

  char* data;
  int retval;
  int hexval;
  assert(bb);
  
  retval = BB_OK;

  if (!strncasecmp(value, "0x", 2)) {
    hexval  = 1;
  } else {
    hexval = 0;
  }

  /* Get address of the data in BB */
  data = bb_item_offset(bb, &data_desc,idxstack,idxstack_len);
  /* Now write the value at obtained offset */
#ifndef __KERNEL__
  retval = bb_value_direct_write(data,data_desc,value,hexval);
#endif
  return retval;
} /* bb_value_write */

#ifndef __KERNEL__


static int32_t
bb_data_header_print_xml(struct bb_printer *bp, S_BB_DATADESC_T data_desc, int32_t idx, int32_t aliastack) {
  const char oneTab[] = "    "; 
  char tabs[MAX_ALIAS_LEVEL*5]="";
  char *n;
  int i;
  struct classic_printer_priv *priv = bp->priv;

  for (i=0; i<(aliastack-1); i++)
    {
      strncat(tabs, oneTab, sizeof(tabs)-strlen(tabs));
    }

  n = bb_get_varname(&data_desc);
  fprintf(priv->fp,"%s<variable name=\"%s\" ", tabs, n);
  free(n);
  fprintf(priv->fp,"alias_target=\"%d\" ", data_desc.alias_target);
  fprintf(priv->fp,"type=\"%d\" ", data_desc.type);
  fprintf(priv->fp,"dimension=\"%d\" ",data_desc.dimension);
  fprintf(priv->fp,"type_size=\"%zd\"  ",data_desc.type_size);
  fprintf(priv->fp,"data_offset=\"%ld\" ",data_desc.data_offset);
  if ((idx>=0) &&
      ((E_BB_UCHAR != data_desc.type) &&
       (E_BB_CHAR  != data_desc.type)
       )
      ) {
    fprintf(priv->fp,"index=\"%d\" value=",idx);
  } else {
    fprintf(priv->fp,"value=");
  }
  return 0;
} /* end of bb_data_header_print */

static int32_t
bb_data_header_print(struct bb_printer *bp, S_BB_DATADESC_T data_desc, int32_t idx, int32_t aliastack) {
  const char oneTab[] = "    "; 
  char tabs[MAX_ALIAS_LEVEL*5]="";
  char *n;
  int i;
  struct classic_printer_priv *priv = bp->priv;

  for (i=0; i<(aliastack-1); i++)
    {
      strncat(tabs, oneTab, sizeof(tabs)-strlen(tabs));
    }

  n = bb_get_varname(&data_desc);
  fprintf(priv->fp,"%s---------- < %s > ----------\n", tabs, n);
  free(n);
  fprintf(priv->fp,"%s  alias-target = %d\n", tabs, data_desc.alias_target);
  fprintf(priv->fp,"%s  type         = %d  (%s)\n",tabs,data_desc.type,E_BB_2STRING[data_desc.type]);
  fprintf(priv->fp,"%s  dimension    = %d  \n",tabs,data_desc.dimension);
  fprintf(priv->fp,"%s  type_size    = %zd  \n",tabs,data_desc.type_size);
  fprintf(priv->fp,"%s  data_offset  = %ld \n",tabs,data_desc.data_offset);
  if ((idx>=0) && 
      ((E_BB_UCHAR != data_desc.type) &&
       (E_BB_CHAR  != data_desc.type)
       )
      ) {
    fprintf(priv->fp,"%s  value[%d]     = ",tabs,idx);
  } else {
    fprintf(priv->fp,"%s  value        = ",tabs);
  }
  if ((data_desc.dimension > 1) &&
      ((E_BB_UCHAR != data_desc.type) &&
       (E_BB_CHAR  != data_desc.type)
       )
      ){
    fprintf(priv->fp," [ ");
  } 

  if ((idx>=0) && 
      ((E_BB_UCHAR != data_desc.type) &&
       (E_BB_CHAR  != data_desc.type)
       )
      ) {
    fprintf(priv->fp,"... ");
  }
  return 0;
} /* end of bb_data_header_print */

static int32_t
bb_data_footer_print_xml(struct bb_printer *bp, S_BB_DATADESC_T data_desc, int32_t idx, int32_t aliastack) {
  struct classic_printer_priv *priv = bp->priv;
  fprintf(priv->fp,"</variable>\n");

  return 0;
} /* end of bb_data_footer_print */

static int32_t
bb_data_footer_print(struct bb_printer *bp, S_BB_DATADESC_T data_desc, int32_t idx, int32_t aliastack) {
  const char oneTab[] = "    ";
  char tabs[MAX_ALIAS_LEVEL*5]="";
  struct classic_printer_priv *priv = bp->priv;
  int i;
  
  for (i=0; i<(aliastack-1); i++) {
    strncat(tabs, oneTab, strlen(oneTab)+sizeof(tabs));
  }
    
  if ((idx>=0) &&
      ((E_BB_UCHAR != data_desc.type) &&
       (E_BB_CHAR  != data_desc.type)
       )
      ) {
    fprintf(priv->fp,"... ");
  }  

  if ((data_desc.dimension > 1) &&
      ((E_BB_UCHAR != data_desc.type) &&
       (E_BB_CHAR  != data_desc.type)
       )
      ){
    fprintf(priv->fp,"]");
  }  
  fprintf(priv->fp,"\n");    
  fprintf(priv->fp,"%s---------- ---------- ----------\n", tabs);
  
  return 0;
} /* end of bb_data_footer_print */

static int32_t 
bb_string_value_print(struct bb_printer *bp, volatile S_BB_T* bb, S_BB_DATADESC_T data_desc,
		      int32_t* idxstack, int32_t idxstack_len) {
  int32_t i,ibeg,iend;
  char* data;
  int32_t idx;
  struct classic_printer_priv *priv = bp->priv;

  assert(bb);
  /* We get BB data address  */
  data = bb_item_offset(bb, &data_desc,idxstack,idxstack_len);
  
  /* check index stack to handle index */
  if ((idxstack_len>0) && (data_desc.dimension > 1)){
    idx = idxstack[idxstack_len-1];
    if (idx>=0) {
      ibeg=0;
      iend=1;
    } else {
      ibeg=0;
      iend=data_desc.dimension;
    }
  } else {
    idx = 0;
    ibeg=0;
    iend=data_desc.dimension;
  }

  /* 
   * If we have character this means we want
   * a character-like or string-like display 
   * HEX dump should be for UINT8/16/32/64
   */
  if ((E_BB_CHAR ==data_desc.type) ||
      (E_BB_UCHAR==data_desc.type)) {
    for (i=ibeg; i< iend; ++i) {
      if ('\0'==((char*) data)[i]) {
	break;
      } else {
	fprintf(priv->fp,"%c",isprint((int)data[i]) ? ((char*) data)[i] : '?');
      }
    }
  } else {
    /* This is not a char type ? */
    fprintf(priv->fp,"bb_string_value_print: Not a char type");
    return 1;
  }
  return 0;
} /* end of bb_string_value_print */

static int32_t
__bb_value_print(struct bb_printer *bp, volatile S_BB_T* bb, S_BB_DATADESC_T data_desc,
		int32_t* idxstack, int32_t idxstack_len, char separator)
{
	int32_t i,j,ibeg,iend;
	char* data;
	int32_t idx;
	int32_t charNullCount;
	struct classic_printer_priv *priv = bp->priv;

	assert(bb);
	/* We get BB data address  */
	data = bb_item_offset(bb, &data_desc,idxstack,idxstack_len);

	/* check index stack to handle index */
	if ((idxstack_len>0) && (data_desc.dimension > 1)){
		idx = idxstack[idxstack_len-1];
		if (idx>=0) {
			ibeg=0;
			iend=1;
		} else {
			ibeg=0;
			iend=data_desc.dimension;
		}
	} else {
		idx = 0;
		ibeg=0;
		iend=data_desc.dimension;
	}

	/* 
	 * If we have character this means we want
	 * a character-like or string-like display 
	 * HEX dump should be for UINT8/16/32/64
	 */
	if ((E_BB_CHAR ==data_desc.type) ||
			(E_BB_UCHAR==data_desc.type)) {
		charNullCount = 0;
		for (i=ibeg; i< iend; ++i) {
			if ('\0'==((char*) data)[i]) {
				charNullCount++;
			} else {
				if (charNullCount>0) {
					fprintf(priv->fp,"(%d*'\\0')",charNullCount);
					charNullCount = 0;
				}
				if (isprint((int)data[i])) {
					fprintf(priv->fp,"%c",((char*) data)[i]);
				} else {
					fprintf(priv->fp," 0x%02X ",((char*) data)[i]);
				}
			}
		}
		if (charNullCount>0 && separator == ' ') {
			fprintf(priv->fp," (%d*'\\0')",charNullCount);
			charNullCount = 0;
		}

		/*     fprintf(priv->fp," (0x"); */
		/*     for (i=ibeg; i< iend; ++i) { */
		/*       fprintf(priv->fp,"%02x",((char*) data)[i]); */
		/*     } */
		/*     fprintf(priv->fp,")"); */
	} else {

		for (i=ibeg; i< iend; ++i) {    
			switch (data_desc.type) {
				case E_BB_DOUBLE: 
					fprintf(priv->fp,"%1.16f",((double*) data)[i]);
					break;
				case E_BB_FLOAT:
					fprintf(priv->fp,"%f",((float*) data)[i]);
					break;
				case E_BB_INT8:
					fprintf(priv->fp,"%d",((int8_t*) data)[i]);
					break;
				case E_BB_INT16:
					fprintf(priv->fp,"%d",((int16_t*) data)[i]);
					break; 
				case E_BB_INT32:
					fprintf(priv->fp,"%d",((int32_t*) data)[i]);
					break; 
				case E_BB_INT64:
					fprintf(priv->fp,"%"BB_INT64_FORMAT"d",((int64_t*) data)[i]);
					break;
				case E_BB_UINT8:
					fprintf(priv->fp,"0x%x",((uint8_t*) data)[i]);
					break; 
				case E_BB_UINT16:
					fprintf(priv->fp,"0x%x",((uint16_t*) data)[i]);
					break;
				case E_BB_UINT32:
					fprintf(priv->fp,"0x%x",((uint32_t*) data)[i]);
					break;	
				case E_BB_UINT64:
					fprintf(priv->fp,"0x%"BB_INT64_FORMAT"x",((uint64_t*) data)[i]);
					break;	
					/*     case E_BB_CHAR: */
					/*       fprintf(priv->fp,"0x%02x<%c> ",((char*) data)[i], */
					/* 	      isprint((int)data[i]) ? ((char*) data)[i] : '?'); */
					/*       break; */
					/*     case E_BB_UCHAR: */
					/*       fprintf(priv->fp,"0x%02x<%c> ",((char*) data)[i], */
					/* 	      isprint((int)data[i]) ? ((char*) data)[i] : '?'); */
					/*       break;*/
				case E_BB_USER:
					for (j=0; j<data_desc.type_size; ++j) {
						fprintf(priv->fp,"0x%02x",((uint8_t*) data)[i*data_desc.type_size+j]);
						if (((j+1)<data_desc.type_size) && separator != ' ') {
							fprintf(priv->fp, "%c", separator);
						}
						//bb_data_print(bb, *(&data_desc+1), priv->fp);  
					}
					break; 
				default:
					fprintf(priv->fp,"0x%x",((char*) data)[i]);
					break;
			}
			if ((i + 1) < iend) {
				fprintf(priv->fp, "%c", separator);
			}
		}
	} 
	return 0;
} /* end of __bb_value_print */

static int32_t 
bb_value_print_xml(struct bb_printer *bp, volatile S_BB_T* bb, S_BB_DATADESC_T data_desc,
	           int32_t* idxstack, int32_t idxstack_len)
{
	struct classic_printer_priv *priv = bp->priv;
	int32_t ret;
	fprintf(priv->fp,"\"");
	ret = __bb_value_print(bp, bb, data_desc, idxstack, idxstack_len, ',');
	fprintf(priv->fp,"\">\n");
	return ret;
}
static int32_t 
bb_value_print(struct bb_printer *bp, volatile S_BB_T* bb, S_BB_DATADESC_T data_desc,
	       int32_t* idxstack, int32_t idxstack_len)
{
	return __bb_value_print(bp, bb, data_desc, idxstack, idxstack_len, ' ');
}
static int32_t
bb_data_print(struct bb_printer *bp, volatile S_BB_T * bb,
	      S_BB_DATADESC_T data_desc, int32_t * idxstack,
	      int32_t idxstack_len)
{
	int32_t aliasstack_size = MAX_ALIAS_LEVEL;
	S_BB_DATADESC_T aliasstack[MAX_ALIAS_LEVEL];

	aliasstack[0] = data_desc;
	bb_find_aliastack(bb, aliasstack, &aliasstack_size);
	bp->ops->bb_data_header_print(bp, data_desc, -1, aliasstack_size);
	bp->ops->bb_value_print(bp, bb, data_desc, idxstack, idxstack_len);
	bp->ops->bb_data_footer_print(bp, data_desc, -1, aliasstack_size);
	return BB_OK;
}				/* end of bb_data_print */
#endif /* !__KERNEL__ */

int32_t 
bb_create(S_BB_T** bb, 
	       const char* pc_bb_name,
	       int32_t n_data,
	       int32_t data_size) {

  enum bb_type type;
  int32_t retcode;
  struct S_BB_LOCAL *local;
  
  assert(bb);
  local = bb_local_new();
  if (local == NULL) {
          retcode = BB_NOK;
          goto err;
  }
  
  type = bb_type(pc_bb_name);
  retcode = ops[type]->bb_shmem_get(bb, local, pc_bb_name, n_data, data_size, 1);
  if (retcode != BB_OK)
  	goto err;

  retcode = ops[(*bb)->type]->bb_sem_get(*bb, 1);
  if (retcode != BB_OK)
  	goto err;

  retcode = ops[(*bb)->type]->bb_msgq_get(*bb, 1);

  bb_attach_local(*bb, local);

err:
  return retcode;
} /* end of bb_create */
#ifdef __KERNEL__
EXPORT_SYMBOL_GPL(bb_create);
#endif

int32_t 
bb_destroy(S_BB_T** bb) {
  
  int32_t retcode;
  struct S_BB_LOCAL *local;
  
  assert(bb);
  assert(*bb);
  
  local = bb_get_local(*bb);
  if (local == NULL) {
          retcode = BB_NOK;
          goto out;
  }

  bb_detach_local(*bb);

  /*
   * On signale la destruction en cours pour les processes qui
   * resteraient attach�s
   */
  (*bb)->status = BB_STATUS_DESTROYED;
   retcode = ops[(*bb)->type]->bb_sem_destroy(*bb);
  if (retcode != BB_OK)
  	goto out;
  retcode = ops[(*bb)->type]->bb_msgq_destroy(*bb);
  if (retcode != BB_OK)
  	goto out;
  retcode = ops[(*bb)->type]->bb_shmem_destroy(bb, local);
  if (retcode != BB_OK)
  	goto out;
  
out:
  bb_local_delete(local);
  return retcode;
} /* end of bb_destroy */
#ifdef __KERNEL__
EXPORT_SYMBOL_GPL(bb_destroy);
#endif


int32_t
bb_data_memset(S_BB_T* bb, const char c) { 
  int32_t retcode;  
  assert(bb);
  retcode = 0;
  memset(bb_data(bb),c,bb->max_data_size);
  return retcode;
} /* end of bb_data_memset */

int32_t 
bb_lock(volatile S_BB_T* bb) {
  return ops[bb->type]->bb_lock(bb);
} /* end of bb_lock */

int32_t 
bb_unlock(volatile S_BB_T* bb) {
  
  return ops[bb->type]->bb_unlock(bb);
} /* end of bb_unlock */

int32_t 
bb_attach(S_BB_T** bb, const char* pc_bb_name) 
{
  enum bb_type type;
  int ret;
  struct S_BB_LOCAL *local;
  
  assert(bb);

  local = bb_local_new();
  if (local == NULL)
          return  BB_NOK;

  type = bb_type(pc_bb_name);
  ret = ops[type]->bb_shmem_attach(bb, local, pc_bb_name);
  if (ret != BB_OK)
    return ret;

  /* older versions don't support name encoding */
  if ((*bb)->bb_version_id < 0x0004000)
    return ret;

#ifndef __KERNEL__
  if (bb_varname_init(*bb) != BB_OK) {
    bb_logMsg(BB_LOG_WARNING, "%s", "Could not setup a proper varname encoding scheme\n");
  }
#endif

  bb_attach_local(*bb, local);

  return ret;
} /* end of bb_attach */

int32_t
bb_detach(S_BB_T** bb) {
	int32_t ret ;
	struct S_BB_LOCAL *local;
	assert(bb);
	assert(*bb);

	local = bb_get_local(*bb);
	if (local == NULL) {
		return BB_NOK;
	}

	bb_detach_local(*bb);

        ret =  ops[(*bb)->type]->bb_shmem_detach(bb, local);

        bb_local_delete(local);

        return ret;
} /* end of bb_detach */

void*
bb_publish(volatile S_BB_T *bb, S_BB_DATADESC_T* data_desc) {
  void* retval;
  int32_t needed_size;
  char *n;

  retval = NULL;
  assert(bb);
  assert(data_desc);

  /* Verify that the published data is not already published
   * (key unicity) and trigger automatic subscribe
   * if key already exists.
   */
  bb_lock(bb);
  n = bb_get_varname(data_desc);
  if (bb_find(bb, n) != -1) {
    char *name = bb_get_varname(data_desc);
    bb_logMsg(BB_LOG_FINER,"BlackBoard::bb_publish",
        "Key <%s> already exists in blackboard (automatic subscribe)!!", name);
    free(name);
    bb_unlock(bb);
    retval = bb_subscribe(bb,data_desc);
    bb_lock(bb);
  } else {
    /* compute required data size  */
    needed_size = data_desc->type_size*data_desc->dimension;
    /* verify available space in BB data descriptor zone */
    if (bb->n_data >= bb->max_data_desc_size) {
      bb_logMsg(BB_LOG_SEVERE,"BlackBoard::bb_publish",
		"No more room in BB data descriptor!! [current n_data=%d]",
		bb->n_data);
      /* verify available space in BB data zone */
    } else if ((bb->max_data_size-bb->data_free_offset) < needed_size) {
      bb_logMsg(BB_LOG_SEVERE,"BlackBoard::bb_publish",
		"No more room in BB data zone!! [left <%d> byte(s) out of <%d> required]",
		bb->max_data_size-bb->data_free_offset,needed_size);
    } else {
      /* Compute the free address */
      retval = (char*) bb_data(bb) + bb->data_free_offset;
      /* Update returned data descriptor */
      data_desc->data_offset = bb->data_free_offset;
      /* this is not an alias */
      data_desc->alias_target = -1;
      /* Update next free address */
      bb->data_free_offset  = bb->data_free_offset + needed_size;
      /* Update data descriptor zone */
      bb_data_desc(bb)[bb->n_data] = *data_desc;
      /* Increment number of published data */
      bb->n_data++;
    }
    /* initialize publish data zone with default value */
    bb_data_initialise(bb,data_desc,NULL);
  }
  free(n);
  /* no init in case of automatic subscribe */
  bb_unlock(bb);
  return retval;
} /* end of bb_publish */

void*
bb_subscribe(volatile S_BB_T *bb, S_BB_DATADESC_T* data_desc) {
	int32_t indexstack[MAX_ALIAS_LEVEL];
  	/* zero out indexstack */
	memset(indexstack,0,MAX_ALIAS_LEVEL*sizeof(int32_t));

	return bb_alias_subscribe(bb,data_desc, indexstack,MAX_ALIAS_LEVEL);
} /* end of bb_subscribe */

void*
bb_item_offset(volatile S_BB_T *bb, 
	       S_BB_DATADESC_T* data_desc,
	       const int32_t* indexstack,
	       const int32_t indexstack_len) {
  
  void* retval;
  S_BB_DATADESC_T  aliasstack[MAX_ALIAS_LEVEL];
  int32_t          aliasstack_size = MAX_ALIAS_LEVEL;
  int32_t          myIndexstack[MAX_ALIAS_LEVEL];
  int32_t          i,j;
  
  retval = NULL;
  assert(bb);
  assert(data_desc);

  /* zero out indexstack */
  memset(myIndexstack,0,MAX_ALIAS_LEVEL*sizeof(int32_t));
  /* We seek the data using its key (name) */

  /* ********** ALIAS publish case **************** */
  if (bb_isalias(data_desc)) {

    /* rebuild alias stack beginning with current symbol */
    aliasstack[0] = *data_desc;
    if (!bb_find_aliastack(bb,aliasstack,&aliasstack_size)) {
      /* fill index stack with missing 0 (scalar case) */
      /* provided stack index is inverted */
      for (i=0,j=indexstack_len-1;i<aliasstack_size;++i) {
	if (1==aliasstack[i].dimension) {
	  myIndexstack[i] = 0;
	} else {
	  /* *** FIXME check provided indexstack length 
	     *** in order to avoid buffer overflow 
	     *** */
	  if (j>=0) {
	    myIndexstack[i] = indexstack[j];
	    --j;
	  } else {
	    myIndexstack[i] = 0;
	  }
	}		
      }
      /* force last index (first on stack) to zero */
      if (data_desc->type == E_BB_USER){
	myIndexstack[0] = 0;
      }
      retval = (char*) bb_data(bb) + 
	bb_aliasstack_offset(aliasstack,myIndexstack,aliasstack_size);	    
    } else {
      bb_logMsg(BB_LOG_SEVERE,
		"Blackboard::bb_item_offset",
		"Cannot resolve alias stack!!");
      retval = NULL;
    }
  }
  /* ********** GENUINE publish case **************** */ 
  else {
    retval = (char*) bb_data(bb) + 
      data_desc->data_offset + 
      /* if indexstack_len is not greater than 0 then indexstack[0] have no sense */ 
      (indexstack_len>0 ? (indexstack[0])*(data_desc->type_size) :  0 ) ;
  }
  return retval;
} /* end of bb_item_offset */


#ifndef __KERNEL__

static int32_t
bb_header_print_xml(struct bb_printer *printer, volatile S_BB_T *bb)
{
	struct classic_printer_priv *priv = printer->priv;
	fprintf(priv->fp,"<bb name=\"%s\" ", bb->name);
	fprintf(priv->fp,"n_data=\"%d\" ", bb->n_data);
	fprintf(priv->fp,"max_data_desc_size=\"%ld\">\n", bb->max_data_size);
	return 0;
}

static int32_t
bb_header_print(struct bb_printer *printer, volatile S_BB_T *bb)
{
	struct classic_printer_priv *priv = printer->priv;
	fprintf(priv->fp,"============= <[begin] BlackBoard [%s] [begin] > ===============\n", bb->name);
	fprintf(priv->fp,"  @start blackboard    = 0x%zx\n",(uintptr_t) (bb));
	fprintf(priv->fp,"  stored data          = %d / %d [max desc]\n",
			bb->n_data,
			bb->max_data_desc_size);
	fprintf(priv->fp,"  free data size       = %ld / %ld\n",
			bb->max_data_size - bb->data_free_offset,
			bb->max_data_size);  
	fprintf(priv->fp,"  @data_desc           = 0x%zx\n",
			(uintptr_t) (bb_data_desc(bb)));
	fprintf(priv->fp,"  @data                = 0x%zx\n",
			(uintptr_t) (bb_data(bb)));
	fprintf(priv->fp,"================ < [begin] Data [begin] > ==================\n");
	return 0;
}

static int32_t
bb_footer_print_xml(struct bb_printer *printer, volatile S_BB_T *bb)
{
	struct classic_printer_priv *priv = printer->priv;
	fprintf(priv->fp,"</bb>\n");
	return 0;
}

static int32_t
bb_footer_print(struct bb_printer *printer, volatile S_BB_T *bb)
{
	struct classic_printer_priv *priv = printer->priv;
	fprintf(priv->fp,"================== < [end] Data [end] > ====================\n");
	fprintf(priv->fp,"============== < [end] BlackBoard [%s] [end] > ================\n",
			bb->name);
	return 0;
}

struct bb_printer_operations xml_printer_ops =  { "xml",
						  bb_data_print,
						  bb_data_footer_print_xml,
						  bb_data_header_print_xml,
						  bb_value_print_xml,
						  bb_string_value_print,
						  bb_header_print_xml,
						  bb_footer_print_xml,
						};
struct bb_printer_operations classic_printer_ops =  { "classic",
						      bb_data_print,
						      bb_data_footer_print,
						      bb_data_header_print,
						      bb_value_print,
						      bb_string_value_print,
						      bb_header_print,
						      bb_footer_print,
						    };
struct bb_printer_operations *printer_operations[] = { &classic_printer_ops,
						       &xml_printer_ops,
						       NULL };

struct bb_printer_operations *get_printer_ops_from_format(char *format)
{
	int i;
	for (i=0; printer_operations[i] != NULL; i++) {
		if (!strncmp(printer_operations[i]->name,
			     format, BB_PRINTER_OPT_NAME_LEN)) {
			return printer_operations[i];
		}
	}
	return NULL;
}
int32_t
bb_dump(volatile S_BB_T *bb, struct bb_printer *printer) {

  int32_t retcode;
  /* char syserr[MAX_SYSMSG_SIZE]; */
  int32_t i;
  int32_t j;
  int32_t indexstack[MAX_ALIAS_LEVEL];
  int32_t indexstack_len;
  int32_t          aliasstack_size = MAX_ALIAS_LEVEL;
  S_BB_DATADESC_T  aliasstack[MAX_ALIAS_LEVEL];
  int32_t array_in_aliasstack;

  retcode = BB_OK;
  assert(bb);
  printer->ops->bb_header_print(printer, bb);
  for (i=0;i<bb->n_data;++i) {
    /* NON ALIAS CASE */
    if ((bb_data_desc(bb)[i]).alias_target == -1) {
      indexstack[0]  = 0;
      indexstack_len = 0;
      printer->ops->bb_data_print(printer, bb,bb_data_desc(bb)[i],indexstack, indexstack_len);
    }
    /* ALIAS CASE */
    else {
      array_in_aliasstack = 0;
      aliasstack[0]=bb_data_desc(bb)[i];
      aliasstack_size = MAX_ALIAS_LEVEL;
      bb_find_aliastack(bb, aliasstack, &aliasstack_size);
      /*		for (j=0; j<aliasstack_size; j++){
	if (aliasstack[j].dimension>1){
	array_in_aliasstack = 1;
	}
	}
	if (!array_in_aliasstack)*/
      if ((aliasstack[aliasstack_size-1].dimension) <= 1) {
	for (j=0; j<bb_data_desc(bb)[i].dimension; j++){
	  indexstack[0] = j;
	  indexstack_len = 1;
	  printer->ops->bb_data_print(printer, bb, bb_data_desc(bb)[i], indexstack, indexstack_len);
	  
	}
      }
      
      else {
	for (j=0; j<aliasstack[aliasstack_size-1].dimension; j++){
	  indexstack[0] = j;
	  indexstack_len = 1;
	  printer->ops->bb_data_print(printer, bb, bb_data_desc(bb)[i], indexstack, indexstack_len);
	  
	}			
      }             
    }
  }
  printer->ops->bb_footer_print(printer, bb);

  return retcode;
} /* end of bb_dump */

#endif /* ! __KERNEL__ */

int32_t
bb_get_nb_max_item(volatile S_BB_T *bb) {
  
  int32_t retval;  
  
  
  retval = -1;
  assert(bb);
  
  retval = bb->max_data_desc_size;
  
  return retval;
}

int32_t
bb_get_nb_item(volatile S_BB_T *bb) {

  
  int32_t retval;  
  
  
  retval = -1;
  assert(bb);
  
  retval = bb->n_data;
  
  return retval;
}

int32_t
bb_get_mem_size(volatile S_BB_T *bb) {

  
  int32_t retval;  
  
  
  retval = -1;
  assert(bb);
  
  retval = bb_size(bb->max_data_desc_size,
			bb->max_data_size);
  
  return retval;
}

int32_t 
bb_shadow_get(S_BB_T *bb_shadow,
	      volatile S_BB_T *bb_src) {
  
  int32_t retcode;

  bb_lock(bb_src);  
  assert(bb_src);
  assert(bb_shadow);
  retcode = BB_OK;
  /* raw copy of BB */
  memcpy(bb_shadow,
	 (void*)bb_src,
	 bb_get_mem_size(bb_src));
  bb_shadow->status = BB_STATUS_SHADOW;
  bb_unlock(bb_src);
  
  return retcode;
}

int32_t 
bb_shadow_update_data(S_BB_T *bb_shadow,
		      volatile S_BB_T *bb_src) {  
  int32_t retcode;
  
  assert(bb_src);
  assert(bb_shadow);
  retcode = BB_OK;
  bb_lock(bb_src);
  /* raw copy of BlackBoard data zone content
   * (in fact only used part of the data zone, 
   *  we do not copy unused part) */
  memcpy(bb_data(bb_shadow),
	 bb_data(bb_src),
	 bb_shadow->data_free_offset);
  bb_unlock(bb_src);
  
  return retcode;
} /* end of bb_shadow_update_data */


ssize_t
bb_snd_msg(volatile S_BB_T *bb,
		S_BB_MSG_T* msg) {
  
  int32_t retcode;
  
  retcode = BB_OK;
  assert(bb);
  return ops[bb->type]->bb_msgq_send(bb, msg);
} /* end of bb_snd_msg */


ssize_t
bb_rcv_msg(volatile S_BB_T *bb,
		S_BB_MSG_T* msg) {
    
  assert(bb);

  return ops[bb->type]->bb_msgq_recv(bb, msg);
} /* end of bb_rcv_msg */


int32_t
bb_get_array_name(char * array_name,
		  int array_name_size_max,
		  S_BB_DATADESC_T * aliasstack, int32_t aliasstack_size,
		  int32_t * indexstack, int32_t indexstack_len) {

  char * part_of_name;
  int32_t indexstack_curr;
  int j;
    
#ifdef __KERNEL__
  part_of_name = kmalloc(array_name_size_max, GFP_KERNEL);
#else
  part_of_name = malloc(array_name_size_max);
#endif /* __KERNEL__ */

  indexstack_curr = 0;
  for (j=aliasstack_size-1; j>=0; j--) {
    /* If this alias is an array */
    if (aliasstack[j].dimension > 1) {
      if (j==aliasstack_size-1) {
        char *n = bb_get_varname(&aliasstack[j]);
        snprintf(part_of_name, array_name_size_max, "%s[%0d]", n,
            indexstack[indexstack_curr]);
        free(n);
      } else {
        char *n1, *n2;
        n1 = bb_get_varname(&aliasstack[j]);
        n2 = bb_get_varname(&aliasstack[j+1]);
        snprintf(part_of_name, array_name_size_max, "%s[%0d]",
            strstr(n1, n2 + strlen(n2)),
            indexstack[indexstack_curr]);
        free(n1);
        free(n2);
      }      
      strncat(array_name, part_of_name, array_name_size_max-strlen(array_name));
      /* go to next index in the index stack */
      indexstack_curr++;
    }
    /* The current alias is a scalar */
    else {
      char *n1, *n2;
      if (j==aliasstack_size-1){
        n1 = bb_get_varname(&aliasstack[j]);
        snprintf(part_of_name, array_name_size_max, "%s", n1);
        free(n1);
      } else {
        n1 = bb_get_varname(&aliasstack[j]);
        n2 = bb_get_varname(&aliasstack[j+1]);
        snprintf(part_of_name, array_name_size_max, "%s",
            strstr(n1, n2 + strlen(n2)));
        free(n1);
        free(n2);
      }
      strncat(array_name, part_of_name, array_name_size_max-strlen(array_name));
    }
  }
  free (part_of_name);
  return BB_OK;

} /* end of get_array_name */

int32_t
bb_msgq_isalive(S_BB_T *bb)
{
	return ops[bb->type]->bb_msgq_isalive(bb);
}

#ifdef __KERNEL__
int32_t 
bb_logMsg(const BB_LOG_LEVEL_T level, const char* who, char* fmt, ...) 
{
	va_list args;	
	char message[2048];

	memset(message,0,2048);
	va_start(args, fmt);
	vsnprintf(message, 2048, fmt, args);
	va_end(args);
	printk("bb: %s : %s", who, message);
	return 0;
}
#else
int32_t 
bb_logMsg(const BB_LOG_LEVEL_T level, const char* who, char* fmt, ...) {
  va_list args;
  char message[2048];
  char *tmp_str;

  memset(message,0,2048);
  va_start(args, fmt);
  vsnprintf(message, 2048, fmt, args);
  va_end(args);
  /* add strerror, if needed */
  if (errno != 0) {
    tmp_str = strdup(message);
    if (!tmp_str) {
      	printf("Cannot allocate memory for message %s\n", message);
	return BB_NOK;
    }
    snprintf(message, 2048, "%s: %s", tmp_str, strerror(errno));

    free(tmp_str);
  }
  
  switch (level) {
    
  case BB_LOG_ABORT:
  case BB_LOG_SEVERE:
    STRACE_ERROR("%s : %s",who,message);
    break;
  case BB_LOG_WARNING:
    STRACE_WARNING("%s : %s",who,message);
    break;
  case BB_LOG_INFO:
  case BB_LOG_CONFIG:
    STRACE_INFO("%s : %s",who,message);
    break;
  case BB_LOG_FINE:
  case BB_LOG_FINER:
    STRACE_DEBUG("%s : %s",who,message);

  }
  return 0;
} /* end of bb_logMsg */

#endif /* __KERNEL__ */


