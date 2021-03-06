/*

$Header: /home/def/zae/tsp/tsp/src/util/libbb/bb_core_k.c,v 1.9 2008-07-24 13:09:11 jaggy Exp $

-----------------------------------------------------------------------

TSP Library - core components for a generic Transport Sampling Protocol.

Copyright (c) 2006 Frederik DEWEERDT

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

Purpose   : Blackboard In-Kernel user and kernel space implementation

-----------------------------------------------------------------------
 */

#ifdef __KERNEL__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/connector.h>

#else

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/connector.h>
#include <linux/netlink.h>
#include <errno.h>

#endif /* __KERNEL__ */

#include "bb_core.h"
#include "bb_core_k.h"
#include "bb_utils.h"
#include "bb_local.h"

#define MOD_NAME "BB_CORE_K"


/* dummy declaration, see the bottom of the file */
struct bb_operations k_bb_ops;
#ifdef __KERNEL__
/** 
 * Array holding the used BBs, this is usefull
 * at unload time, where we want to clean up every
 * in-use bb before leaving
 */
S_BB_T *present_bbs[BB_DEV_MAX];
/** Tracks the used/unused BBs */
DECLARE_BITMAP(present_devices, BB_DEV_MAX);
/** protects access to the two bb tracking tools above */
DEFINE_SPINLOCK(pdeviceslock);

#endif /* __KERNEL__ */


static int k_bb_msgq_get(S_BB_T * bb, int create)
{
	return BB_OK;
}

#ifdef __KERNEL__
static int k_bb_lock(volatile S_BB_T * bb)
{
	return BB_OK;
}

static int k_bb_unlock(volatile S_BB_T *bb)
{
	return BB_OK;
}

#else

static int k_bb_lock(volatile S_BB_T * bb)
{
	return BB_OK;
}

static int k_bb_unlock(volatile S_BB_T *bb)
{
	return BB_OK;
}

#endif

static int k_bb_sem_get(S_BB_T * bb, int create)
{
	return BB_OK;
}

static int k_bb_shmem_detach(S_BB_T ** bb, struct S_BB_LOCAL *local)
{
	return BB_OK;
}

#ifdef __KERNEL__
static int k_bb_shmem_attach(S_BB_T ** bb, struct S_BB_LOCAL *local,
                             const char *name)
{
	return BB_NOK;
}
#else

#ifdef  ALLOW_KBB_WRITE_ACCESS_FROM_USERSPACE
#define MMAP_PROT (PROT_READ | PROT_WRITE)
#else /* ALLOW_KBB_WRITE_ACCESS_FROM_USERSPACE */
#define MMAP_PROT (PROT_READ)
#endif /* ALLOW_KBB_WRITE_ACCESS_FROM_USERSPACE */

static int k_bb_shmem_attach(S_BB_T ** bb, struct S_BB_LOCAL *local,
                             const char *name)
{
	int fd;
	int shm_size;

        fd = open(name, O_RDWR | O_SYNC);
        if( fd == -1) {
		return BB_NOK;
        }

	/* At first, only map the BB structure ... */
        *bb = mmap(0, sizeof(S_BB_T), PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
        if(*bb == MAP_FAILED) {
		close(fd);
		return BB_NOK;
        }
	shm_size = (*bb)->priv.k.shm_size;
	munmap(*bb, sizeof(S_BB_T));

	/* ... then remap with the size of the BB + _data_ */
        *bb = mmap(0, shm_size, MMAP_PROT, MAP_FILE | MAP_SHARED, fd, 0);
        if(*bb == MAP_FAILED) {
		close(fd);
		return BB_NOK;
        }

	return BB_OK;
}
#endif /* __KERNEL__ */

#ifdef __KERNEL__
static int allocate_bb(S_BB_T ** bb, struct S_BB_LOCAL *local,
                       const char *name, int n_data,
                       int data_size, int create)
{
	int shm_size;
	void *kmalloc_ptr;
	unsigned long virt_addr;

	shm_size = bb_size(n_data, data_size) + PAGE_SIZE;
	/* 
	 * create the shared memory pool. we add two pages to get 
	 * the desired amount of memory aligned on a PAGE boundary,
	 * even in the worst case
	 */
	kmalloc_ptr = kmalloc(shm_size + 2 * PAGE_SIZE, GFP_ATOMIC);
	if (!kmalloc_ptr) {
		printk("kmalloc failed\n");
		return BB_NOK;
	}
	/* Align the kmallocated area on a page boundary */
	*bb =
	    (S_BB_T *) (((unsigned long) kmalloc_ptr + PAGE_SIZE - 1)
			      & PAGE_MASK);
	/* reserve all pages to make them remapable */
	for (virt_addr = (unsigned long) *bb;
	     virt_addr < (unsigned long) *bb + shm_size;
	     virt_addr += PAGE_SIZE) {
		SetPageReserved(virt_to_page(virt_addr));
	}
        if (create) {
                /* Init the freshly allocated structure */
                memset(*bb, 0, shm_size);
                (*bb)->bb_version_id = BB_VERSION_ID;
                strncpy((*bb)->name, name,
                        BB_NAME_MAX_SIZE + 1);
                (*bb)->max_data_desc_size = n_data;
                (*bb)->data_desc_offset = sizeof(S_BB_T);

                (*bb)->n_data = 0;
                (*bb)->max_data_size = data_size;
                (*bb)->data_offset = (*bb)->data_desc_offset +
                    ((*bb)->max_data_desc_size) * sizeof(S_BB_DATADESC_T);
                (*bb)->data_free_offset = 0;
                (*bb)->status = BB_STATUS_GENUINE;
                (*bb)->priv.k.shm_size = shm_size;
                local->kmalloc_ptr = kmalloc_ptr;
        }
        (*bb)->type = BB_KERNEL;
	return BB_OK;
}

static int k_bb_shmem_get(S_BB_T ** bb, struct S_BB_LOCAL *local,
                          const char *name, int n_data,
                          int data_size, int create)
{
	struct bb_device *bb_dev;
	int err, index;
	struct class_device *class;

	/* register and allocate a new character device */
	bb_dev = kmalloc(sizeof(*bb_dev), GFP_KERNEL);
	if (!bb_dev) {
		printk("Not enough memory to allocate device\n");
		return BB_NOK;
	}

	cdev_init(&bb_dev->cdev, &bb_fops);
	bb_dev->cdev.owner = THIS_MODULE;
	bb_dev->cdev.ops = &bb_fops;

	spin_lock(&pdeviceslock);
	index = find_first_zero_bit(present_devices, BB_DEV_MAX);
	if (index >= BB_DEV_MAX) {
		printk("No BBs left :%d\n", index);
		goto err_unlock;
	}
	set_bit(index, present_devices);
	err = allocate_bb(bb, local, name, n_data, data_size, create);
	if (err != BB_OK) {
		printk("Cannot allocate BB\n");
		goto err_unlock;
	}
	present_bbs[index] = *bb;
	spin_unlock(&pdeviceslock);

	bb_dev->devno = MKDEV(bb_major, bb_minor + index);
	err = cdev_add(&bb_dev->cdev, bb_dev->devno, 1);
	if(err != 0)
		goto err_destroy_bb;
	bb_dev->name = strdup(name);

	if (bb_dev->name == NULL)
		goto err_destroy_cdev;

	class = class_device_create(bb_dev_class, NULL, bb_dev->devno,
				    NULL, bb_dev->name);

	if (IS_ERR(class)) {
		printk ("class_device_create \"%s\" FAILED with errno %ld\n",
			name, PTR_ERR(class));
		goto err_free_name_mem;
	}

	printk("New bb device created, major: %d, minor: %d name: %s\n",
	       MAJOR(bb_dev->devno), MINOR(bb_dev->devno), bb_dev->name);
	bb_dev->bb = *bb;
	local->dev = bb_dev;
	(*bb)->priv.k.index = index;

	return BB_OK;

err_free_name_mem:
	kfree(bb_dev->name);
err_destroy_cdev:
	cdev_del (&bb_dev->cdev) ;
err_destroy_bb:
	bb_destroy(bb);
err_unlock:
	spin_unlock(&pdeviceslock);
	kfree(bb_dev);
	return BB_NOK;
}
#else
static int k_bb_shmem_get(S_BB_T ** bb, struct S_BB_LOCAL *local,
                          const char *name, int n_data,
                          int data_size, int create)
{
	return BB_OK;
}
#endif /* __KERNEL__ */


#ifdef __KERNEL__
static int32_t k_bb_shmem_destroy(S_BB_T ** bb, struct S_BB_LOCAL *local)
{
	unsigned long virt_addr;

	class_device_destroy(bb_dev_class, local->dev->devno);
	kfree(local->dev->name);
	cdev_del(&local->dev->cdev);
	spin_lock(&pdeviceslock);
	clear_bit((*bb)->priv.k.index, present_devices);
	spin_unlock(&pdeviceslock);

	for (virt_addr = (unsigned long) *bb;
	     virt_addr < (unsigned long) *bb + (*bb)->priv.k.shm_size;
	     virt_addr += PAGE_SIZE) {
		/* clear all pages */
		ClearPageReserved(virt_to_page(virt_addr));
	}
	kfree(local->kmalloc_ptr);
	*bb = NULL;

	return BB_OK;
} /* end of k_bb_shmem_destroy */
#else
static int32_t k_bb_shmem_destroy(S_BB_T ** bb, struct S_BB_LOCAL *local)
{
	return BB_OK;
}
#endif /* __KERNEL__ */

static int32_t k_bb_msgq_recv(volatile S_BB_T * bb, S_BB_MSG_T * msg)
{
#ifdef __KERNEL__
	return BB_NOK;
#else /* __KERNEL__ */
	int s;
	struct sockaddr_nl l_local;
	char buf[MAX_SYSMSG_SIZE + sizeof(struct nlmsghdr) + sizeof(struct cn_msg)];
	struct nlmsghdr *nlmsghdr;
	struct cn_msg *cn_msg;
	int len;
	int recv_ok = 0;

	/* mtype si stored on the NBIT_FOR_MTYPE LSB bits of cb_id.val */
	if (msg->mtype > MTYPE_MASK)
		return BB_NOK;

	s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (s == -1) {
		bb_logMsg(BB_LOG_SEVERE,"socket failed: ",
			  strerror (errno));
		return BB_NOK;
	}
	l_local.nl_family = AF_NETLINK;
	l_local.nl_groups = 1 << (CN_IDX_BB - 1); /* bitmask of requested groups */
	l_local.nl_pid = 0;

	if (bind(s, (struct sockaddr *)&l_local, sizeof(struct sockaddr_nl)) == -1) {
		bb_logMsg(BB_LOG_SEVERE,"bind failed: ",
			  strerror (errno));
		close(s);
		return -1;
	}

	memset(buf, 0, sizeof(buf));

	while (!recv_ok) {
		len = recv(s, buf, sizeof(buf), 0);
		if (len == -1) {
			bb_logMsg(BB_LOG_SEVERE,"recv failed: ",
				  strerror (errno));
			close(s);
			return BB_NOK;
		}

		nlmsghdr = (struct nlmsghdr *)buf;

		if (nlmsghdr->nlmsg_type != NLMSG_DONE) {
			bb_logMsg(BB_LOG_SEVERE,"%s", "Error message received.");
			continue;
		}

		cn_msg = (struct cn_msg *)NLMSG_DATA(nlmsghdr);

		if (cn_msg->id.val !=
		    MTYPE_INDEX_2_VAL(msg->mtype, bb->priv.k.index)) {
			continue;
		}

		if (cn_msg->id.idx != CN_IDX_BB) {
			continue;
		}

		if (cn_msg->len > MAX_SYSMSG_SIZE) {
			bb_logMsg(BB_LOG_SEVERE, MOD_NAME, "message too long [%d]\n",
				  cn_msg->len);
			close (s);
			return BB_NOK;
		}
		memcpy(msg->mtext, cn_msg->data, cn_msg->len);
		recv_ok = 1;
	}


	close(s);
	return BB_OK;
#endif /* __KERNEL__ */
	}

#ifndef __KERNEL__
static int netlink_send(int s, struct cn_msg *msg);
#endif /* __KERNEL__ */

static int32_t k_bb_msgq_send(volatile S_BB_T * bb, S_BB_MSG_T * msg)
{
#ifdef __KERNEL__
	char buf[MAX_SYSMSG_SIZE + sizeof(struct cn_msg)];

	struct cn_msg *cn_msg = (struct cn_msg *)buf;
	int ret;

	/* mtype si stored on the NBIT_FOR_MTYPE LSB bits of cb_id.val */
	if (msg->mtype > MTYPE_MASK)
		return BB_NOK;

	cn_msg->id.idx = CN_IDX_BB;
	cn_msg->id.val = MTYPE_INDEX_2_VAL(msg->mtype, bb->priv.k.index);
	cn_msg->seq = 0;
	cn_msg->ack = 0;
	cn_msg->len = MAX_SYSMSG_SIZE;

	memcpy (cn_msg->data, msg->mtext, MAX_SYSMSG_SIZE);

	ret = cn_netlink_send(cn_msg, CN_IDX_BB, gfp_any());

	if ( ret < 0 && ret != -ESRCH) {
		printk ("BB %s : Sending idx [%d] val[0x%08x] FAILED "
			"with errno %d\n", bb->name,
			cn_msg->id.idx, cn_msg->id.val, ret);
		return BB_NOK;
	}

	return BB_OK;
#else /* __KERNEL__ */
	int s;
	struct sockaddr_nl l_local;
	char buf[MAX_SYSMSG_SIZE + sizeof(struct cn_msg)];
	struct cn_msg *cn_msg;
	int len;

	s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (s == -1) {
		bb_logMsg(BB_LOG_SEVERE,"socket failed: ",
			  strerror (errno));
		return BB_NOK;
	}
	l_local.nl_family = AF_NETLINK;
	l_local.nl_groups = 1 << (CN_IDX_BB -1); /* bitmask of requested groups */
	l_local.nl_pid = 0;

	if (bind(s, (struct sockaddr *)&l_local, sizeof(struct sockaddr_nl)) == -1) {
		bb_logMsg(BB_LOG_SEVERE,"bind failed: ",
			  strerror (errno));
		close(s);
		return -1;
	}

	cn_msg = (struct cn_msg *)buf;

	cn_msg->id.idx = CN_IDX_BB;
	cn_msg->id.val = MTYPE_INDEX_2_VAL(msg->mtype, bb->priv.k.index);
	cn_msg->seq = 0;
	cn_msg->ack = 0;
	cn_msg->len = MAX_SYSMSG_SIZE;

	memcpy (cn_msg->data, msg->mtext, MAX_SYSMSG_SIZE);

	len = netlink_send(s, cn_msg);

	if (len == -1) {
		bb_logMsg(BB_LOG_SEVERE,"send failed: ",
			  strerror (errno));
		close(s);
		return BB_NOK;
	}
	close(s);
	return BB_OK;
#endif /* __KERNEL__ */
}

static int32_t k_bb_msgq_isalive(S_BB_T * bb)
{
	return BB_OK;
}

static int k_bb_sem_destroy(S_BB_T *bb)
{
	return BB_OK;
}

static int k_bb_msgq_destroy(S_BB_T *bb)
{
	return BB_OK;
}

struct bb_operations k_bb_ops = {
	.bb_shmem_get = k_bb_shmem_get,
	.bb_shmem_attach = k_bb_shmem_attach,
	.bb_shmem_detach = k_bb_shmem_detach,
	.bb_shmem_destroy = k_bb_shmem_destroy,

	.bb_sem_get = k_bb_sem_get,
	.bb_lock = k_bb_lock,
	.bb_unlock = k_bb_unlock,
	.bb_sem_destroy = k_bb_sem_destroy,

	.bb_msgq_get = k_bb_msgq_get,
	.bb_msgq_send = k_bb_msgq_send,
	.bb_msgq_recv = k_bb_msgq_recv,
	.bb_msgq_isalive = k_bb_msgq_isalive,
	.bb_msgq_destroy = k_bb_msgq_destroy,
};


#ifndef __KERNEL__
static int seq = 0;
static int netlink_send(int s, struct cn_msg *msg)
{
	struct nlmsghdr *nlh;
	unsigned int size;
	int err;
	char buf[CONNECTOR_MAX_MSG_SIZE];
	struct cn_msg *m;

	size = NLMSG_SPACE(sizeof(struct cn_msg) + msg->len);

	nlh = (struct nlmsghdr *)buf;
	nlh->nlmsg_seq = seq++;
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_type = NLMSG_DONE;
	nlh->nlmsg_len = NLMSG_LENGTH(size - sizeof(*nlh));
	nlh->nlmsg_flags = 0;

	m = NLMSG_DATA(nlh);

	memcpy(m, msg, sizeof(*m) + msg->len);

	err = send(s, nlh, size, 0);
	if (err == -1) {
		bb_logMsg(BB_LOG_SEVERE, "Failed to send: %s [%d].\n",
			  strerror(errno), errno);
	}

	return err;
}

#endif /* __KERNEL__ */
