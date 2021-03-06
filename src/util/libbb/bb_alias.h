/*

$Header: /home/def/zae/tsp/tsp/src/util/libbb/bb_alias.h,v 1.6 2007-07-24 23:30:12 erk Exp $

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

Purpose   : BlackBoard Idiom implementation

-----------------------------------------------------------------------
 */
#ifndef _BB_ALIAS_H_
#define _BB_ALIAS_H_

#include "bb_core.h"

/** 
 * @addtogroup BBAliasLib 
 * @{
 */

/**
 * The character used for alias separator
 * when generating alias names
 */
#define ALIAS_SEPARATOR     "."
/**
 * The maximum level of <em> alias-of-alias </em>.
 */
#define MAX_ALIAS_LEVEL     10

/** @} */

#ifndef __KERNEL__
BEGIN_C_DECLS
#endif

/**
 * @defgroup BBAliasLib The BB Alias System
 * The BB alias system is a part of the BlackBoard library
 * which enable a special publish interface called
 * ::bb_alias_publish. When publishing alias, one does not
 * allocate new data in the BlackBoard but only define aliases
 * on already published data. Using alias publish one may publish
 * user define type/structure (see #E_BB_USER) and then 
 * describe the elements of the published structure.
 * A <em> normal </em> publish is called a <em>genuine</em>
 * publish which is opposed to an <em> alias publish </em>.
 * Since an alias published data is refering another published
 * data the refered data is called the <em> target </em>
 * of the alias. A target may itself be an alias or a genuine
 * published data.  
 * @ingroup BBLib
 * @{
 */

/**
 * True if the described data is an alias.
 * @param[in] data_desc the data descriptor.
 * @return 1 if the specified data is an alias 0 otherwise.
 */
int32_t bb_isalias(const S_BB_DATADESC_T* data_desc);

/**
 * Build up an alias stack.
 * An alias stack is a sequence of #S_BB_DATADESC_T
 * describing the way to go from an alias to a genuine
 * published symbol.
 * @param[in] bb the BlackBoard
 * @param[in,out] data_desc_stack the array containing the alias stack.
 *           <ul>
 *               <li>IN, the first index must contain the data descriptor for which we are seeking the alias stack </li>
 *               <li>OUT, the first index is not touched and other indexes are updated with intermediate to final target of the alias </li>
 *           </ul>
 * @param[in,out] stack_max_size  Stack maximum size
 *           <ul>
 *               <li>IN, the maximum size of the alias stack array </li>
 *               <li>OUT, the size of the found alias stack </li>
 *           </ul>
 * @return E_OK on sucess E_NOK if we failed to build the alias stack.
 */
int32_t bb_find_aliastack(volatile S_BB_T* bb,       
			  S_BB_DATADESC_T* data_desc_stack, 
			  int32_t* stack_max_size);


/**
 * Compute the offset of the data described
 * by the alias stack.
 * @param[in] data_desc_stack
 * @param[in] index_stack
 * @param[in] stack_size
 * @return the computed offset
 */
unsigned long bb_aliasstack_offset(S_BB_DATADESC_T* data_desc_stack, 
				   int32_t* index_stack,
				   int32_t stack_size);
/**
 * Publish an aliased data in a BlackBoard.
 * @param[in,out] bb  BB pointer (should not be NULL).
 * @param[in,out] data_desc Data descriptor of the data to be published.
 *                  <ul>
 *                         <li> OUT, if data has been properly allocated
 *                         the S_BB_DATADESC_T.data_offset is updated
 *                         and S_BB_DATADESC_T.alias_target too.</li>
 *                  </ul>
 * @param[in] data_desc_target Data descriptor of the data to be aliased.
 * @return address of the (first) aliased data, NULL
 *         if alias failed.
 */
void* 
bb_alias_publish(volatile S_BB_T *bb, S_BB_DATADESC_T* data_desc, 
		 const S_BB_DATADESC_T* data_desc_target);


/**
 * Subscribe to an aliased data in a BlackBoard.
 * @param[in] bb BB pointer (should not be NULL).
 * @param[in,out] data_desc Data descriptor of the data to subscribe to.
 *                  <ul>
 *                         <li>IN, the name of the data is used</li>
 *                         <li>OUT, if data has been properly found
 *                         the data_offset is updated
 *                         and alias_target too.</li>
 *                 </ul>
 * @param[in] indexstack The index stack to be used to find the aliased data
 * @param[in] indexstack_len the length of the indexstack.
 * @return address of the (first) aliased data, NULL
 *         if subscribe failed.
 */
void* 
bb_alias_subscribe(volatile S_BB_T *bb, 
		   S_BB_DATADESC_T* data_desc,
		   const int32_t* indexstack,
		   const int32_t indexstack_len);

/**
 * Increment indexstack from the left most
 * element (alias) to the right most element (target)
 * @param[in]  aliasstack 
 * @param[in]  aliasstack_size
 * @param[in,out]  idxstack
 * @param[in]  idxstack_len
 * @return E_OK on success, E_NOK otherwise
 */					
int32_t 
bb_alias_increment_idxstack(S_BB_DATADESC_T * aliasstack, int32_t aliasstack_size,
			    int32_t * idxstack, int32_t idxstack_len); 


/** @} */ 

#ifndef __KERNEL__
END_C_DECLS
#endif
#endif /* _BB_ALIAS_H_ */
