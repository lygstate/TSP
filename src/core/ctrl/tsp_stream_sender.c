/*

$Header: /home/def/zae/tsp/tsp/src/core/ctrl/tsp_stream_sender.c,v 1.29 2008-11-12 12:32:40 erk Exp $

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
Component : Provider

-----------------------------------------------------------------------

Purpose   : Implementation for the low level functions that send the data
stream  to the consumers. 
-----------------------------------------------------------------------
 */

#include <signal.h>
#include <pthread.h>
#ifdef _WIN32
    #include <Windows.h>
    #include <WinSock2.h>
    #include <ws2tcpip.h>
    #define assert(exp)     ((void)0)
#else
    #include <netdb.h>
    #include <unistd.h>  
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <sys/socket.h>
    #include <assert.h>
#endif

#if !defined (VXWORKS) && !defined(_WIN32) && !defined(__rtems__)
#include <strings.h> /* for bzero */
#endif
#include <string.h>  /* for bzero too :=} */

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

#include "tsp_sys_headers.h"
#include "tsp_stream_sender.h"
#include "tsp_time.h"

#define TSP_DATA_ADDRESS_STRING_SIZE 256

/* (�s) */
#define TSP_STREAM_SENDER_CONNECTION_POLL_TIME ((int)(1e5))

/* (�s) */
#define TSP_STREAM_SENDER_FIFO_POLL_TIME       ((int)(1e5))

struct TSP_socket_t
{
  int socketId;
  int hClient;
    
  /** String for the data address ex: Myhost:Port */
  char data_address[TSP_DATA_ADDRESS_STRING_SIZE + 1];

  /**
   * client connection status.
   * If TRUE, the client is connected to the socket
   */
  int client_is_connected;

  /**
   * Fifo for outgoing data
   */
  TSP_stream_sender_ringbuf_t* out_ringbuf;

  /**
   * Buffer for outgoing data
   */
  TSP_stream_sender_item_t* out_item;

  /** 
   * When fifo size is > 0, a thread is created to send data
   */
  int fifo_size;

  /** 
   * Buffer size to prepare data for the socket
   */
  int buffer_size;


  /** 
   * Thread used when fifo size > 0
   */
  pthread_t thread_id;

  /** 
   * Tell if the stop function was called 
   */
  int is_stopped;

  /** 
   * When the send fails, connection_ok = FALSE;
   */
  int connection_ok;


};

typedef struct TSP_socket_t TSP_socket_t;

/* Signal functions */
#if defined (_WIN32)
    /* no SIGPIPE signal uner Windows (waitforsingle obect insteed) */
#else
typedef void Sigfunc(int);

Sigfunc* MySignal(int signo, Sigfunc* func)
{    
  struct sigaction act, oact;
  
  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if(signo == SIGALRM )
    {
#ifdef SA_INTERRUPT
      act.sa_flags |= SA_INTERRUPT; /*SunOS 4;x */
#endif
    }
  else
    {
#ifdef SA_RESTART
      act.sa_flags |= SA_RESTART; /*SVR4, 4.4BSD*/
#endif
    }
  if(sigaction(signo, &act, &oact) < 0)
    return(SIG_ERR);
  return (oact.sa_handler);
} 
#endif

void* TSP_streamer_sender_thread_sender(void* arg)
{
  TSP_socket_t* sock = (TSP_socket_t*)arg;
  TSP_stream_sender_item_t* item = 0;
  int connection_ok = TRUE;
  static int status = 0;
     
  /* Wait for consumer connection before we send data */  

  STRACE_DEBUG("Thread stream sender created : waiting for client to connect...");
  while(!sock->client_is_connected)
    {
      tsp_usleep(TSP_STREAM_SENDER_CONNECTION_POLL_TIME);
     
    }
  STRACE_DEBUG("Client connected ! Send loop starts !");

  item = RINGBUF_PTR_GETBYADDR(sock->out_ringbuf);
  /* FIXME : handle the stop case cleanly */
  /*         hanlde unable to send case   */
  while(connection_ok && !sock->is_stopped)
    {
      while (item && connection_ok)
	{
	 
	  connection_ok =  TSP_stream_sender_send(sock, TSP_STREAM_SENDER_ITEM_BUF(item), item->len);
	  RINGBUF_PTR_GETBYADDR_COMMIT(sock->out_ringbuf);
	  item = RINGBUF_PTR_GETBYADDR(sock->out_ringbuf);      

	}
      	  tsp_usleep(TSP_STREAM_SENDER_FIFO_POLL_TIME);
	  item = RINGBUF_PTR_GETBYADDR(sock->out_ringbuf);      
    }

  if(!sock->is_stopped)
    {
      STRACE_DEBUG("Connection with client was lost ! ");
    }

  STRACE_DEBUG("End of fifo thread stream sender");
  return &status;
}


/**
* When a fifo is needed create a fifo and a thread to send the data from the fifo 
* @param sock The data stream object instance
* @return OK or KO. TRUE = OK
*/
static int TSP_stream_sender_init_bufferized(TSP_socket_t* sock)
{

  int status;

  RINGBUF_PTR_INIT(TSP_stream_sender_ringbuf_t,
		   sock->out_ringbuf,
		   TSP_stream_sender_item_t, 
		   sock->buffer_size,
		   RINGBUF_SZ(sock->fifo_size) );
  
  /* Memory Exhausted */
  if (NULL==sock->out_ringbuf) {    
    return FALSE;
  }
  assert(sock->out_ringbuf);

  status = pthread_create(&sock->thread_id, NULL, TSP_streamer_sender_thread_sender,  sock);
  TSP_CHECK_THREAD(status, FALSE);
 
  return TRUE;
  
}

static void TSP_stream_sender_save_address_string(TSP_socket_t* sock, 
					     char* host, unsigned short port)
{   
  char strPort[10];
    
  sprintf(strPort,"%u",(int)port);
  strcpy(sock->data_address, host);
  strcat(sock->data_address, ":");    
  strcat(sock->data_address, strPort);
    
  STRACE_DEBUG_MORE("address='%s'", sock->data_address);
}

static void* TSP_streamer_sender_connector(void* arg)
{
  /* FIXME : When the client can't find us (somehow, ex : routing error on the network)
     this function badly leaks */
    
  TSP_socket_t* sock = (TSP_socket_t*)arg;
  socklen_t Len = 0;
  sock->client_is_connected = FALSE;
    
  pthread_detach(pthread_self());
    
  /* Accept connection on socket */
  STRACE_DEBUG("Thread acceptor started waiting for client to connect %d", sock->hClient);
#if defined (_WIN32)  
  sock->hClient = accept(sock->socketId, NULL, NULL);
#else
  /* FIXME */
  sock->hClient = accept(sock->socketId, NULL, &Len);
#endif

  if(sock->hClient > 0) {
    /* OK, the client is connected */
    sock->client_is_connected = TRUE;
    STRACE_DEBUG("New connection accepted on socket client socket %d", sock->hClient);
  }
  else {
    STRACE_ERROR("Accept error");
    close(sock->socketId);
    return 0;
  }
    
  return 0;
}

const char* TSP_stream_sender_get_data_address_string(TSP_stream_sender_t sender)
{   
    
  TSP_socket_t* sock = (TSP_socket_t*)sender;
    
  STRACE_INFO("-->address='%s'", sock->data_address);

  return sock->data_address;
}

TSP_stream_sender_t TSP_stream_sender_create(int fifo_size, int buffer_size)
{

  int status = 0;
  int OptInt = 0;
  char host[TSP_MAXHOSTNAMELEN+1];
  TSP_socket_t* sock;
  pthread_t thread_connect_id;
  struct hostent*  myhost;
  union {
    uint32_t    addr;
    uint8_t     parts[4];
  } myu;

#if defined (_WIN32)
  /* no SIGPIPE signal under Windows (waitforsingleobject insteed) */
#else
  /* First disable SIGPIPE signal to avoir being crashed by a disconnected client*/
  if( SIG_ERR == MySignal(SIGPIPE, SIG_IGN)) {
    STRACE_ERROR("Unable to disable SIGPIPE signal");
    return 0;
  }
#endif
  if( -1 == gethostname(host, TSP_MAXHOSTNAMELEN))
    {
      STRACE_ERROR("gethostname error");

      return 0;
    }
  myhost = gethostbyname(host);
  if (myhost == NULL) {
    STRACE_ERROR("Cannot gethostbyname '(hostname --> @IP)' for host <%s> check your /etc/hosts file.",host);
    /* be tolerant we keep going with hostname but... consumer may not handle this properly */    
  } else {
     /* 
      * Now translate hostname to @IP in order
      * to avoid name -> IP resolution on consumer side 
      */
    memset(host,0,sizeof(host));
    myu.addr =*((uint32_t*)myhost->h_addr_list[0]);
    sprintf(host,"%d.%d.%d.%d",myu.parts[0], myu.parts[1], myu.parts[2], myu.parts[3]);

  }
     
  sock = (TSP_socket_t*)calloc(1, sizeof(TSP_socket_t));
  TSP_CHECK_ALLOC(sock, 0);

  sock->hClient = 0;
  sock->socketId = 0;
  sock->fifo_size = fifo_size;
  sock->buffer_size = buffer_size;
  sock->out_ringbuf = NULL;
  sock->client_is_connected = FALSE;
  sock->is_stopped = FALSE;
  sock->connection_ok = TRUE;
  
  /* Init socket */
  sock->socketId = socket(AF_INET, SOCK_STREAM, 0);

  if (sock->socketId > 0)
    {
      OptInt = TSP_DATA_STREAM_SOCKET_FIFO_SIZE;
      status = setsockopt(sock->socketId, SOL_SOCKET, SO_SNDBUF, (void * )&OptInt, sizeof(OptInt));
      if (status == -1)
	{
	  STRACE_ERROR("Problem with set socket size");

	  close(sock->socketId);
	  return 0;
	}
 
      /* Local address reuse */

      OptInt = 1;
      status = setsockopt(sock->socketId, SOL_SOCKET, SO_REUSEADDR,
			  (void *) &OptInt, sizeof(OptInt));
      if (status == -1)
	{
          STRACE_ERROR("pb set local address reuse");
	  close(sock->socketId);
	  return 0;
	}

      /* No periodic connection state control */
      OptInt = 0;
      status = setsockopt(sock->socketId, SOL_SOCKET, SO_KEEPALIVE,
			  (void *) &OptInt, sizeof(OptInt));
      if (status == -1)
	{
	  STRACE_ERROR("pb set periodic state control");

	  close(sock->socketId);
	  return 0;
	}

      /* TCP No delay */
      OptInt = 1;
      status = setsockopt(sock->socketId, IPPROTO_TCP, TCP_NODELAY, (void *) &OptInt,
			  sizeof(OptInt));
      if (status == -1)
	{
	  STRACE_ERROR("pb set TCP no delay");

	  close(sock->socketId);
	  return 0;
	}

      /* Bind to socket : */
      {

	struct sockaddr_in ListenAddr;

	/* Bind to socket */
	bzero((char *) &ListenAddr, sizeof(struct sockaddr_in));
	ListenAddr.sin_family = AF_INET;
#ifdef _SOCKADDR_LEN
	ListenAddr.sin_len = sizeof(ListenAddr);
#endif
	ListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* The kernel will choose a free port */
	status = bind(sock->socketId, (struct sockaddr*) &ListenAddr, sizeof (struct sockaddr_in));

	if (status != -1)
	  {
	    /* Find the chosen port*/
	    struct sockaddr_in buf_addr;
	    socklen_t len = sizeof(buf_addr);
	    if( 0 ==  getsockname(sock->socketId, (struct sockaddr*) &buf_addr, &len) )
	      {
		/* Save the address in the tsp_socket struct */
		unsigned short port =  ntohs(buf_addr.sin_port);
		TSP_stream_sender_save_address_string(sock, host, port);
	      }
	    else
	      {
		STRACE_ERROR("getsockname failed");
		  
		close(sock->socketId);
		return 0;
	      }
	  }
	else
	  {
	    STRACE_ERROR("pb bind to socket");
	    
	    close(sock->socketId);
	    return 0;
	  }
      }
      

      /* Try and listen onto socket */
      status = listen (sock->socketId, SOMAXCONN);
      if (status == -1)
	{
	  STRACE_ERROR("pb listening to socket");

	  close(sock->socketId);
	  return 0;
	}

      /* If we want a bufferized connection, create a fifo, and launch the sender
	 thread that will read the fifo */
      if(sock->fifo_size > 0 )	
	{
	  if(!TSP_stream_sender_init_bufferized(sock))
	    {
	      STRACE_ERROR("Function TSP_stream_sender_init_bufferized failed");
	      close(sock->socketId);	  
	      return 0;
	    }
	  
	}
      else
	{
	  /* Only create a buffer, with no ringbuffer */
	  sock->out_item = (TSP_stream_sender_item_t*)calloc(1, sizeof(TSP_stream_sender_item_t) + sock->buffer_size);
	  TSP_CHECK_ALLOC(sock->out_item, 0);
	}
      
      /* When the client is be connected, the thread function
	 TSP_streamer_sender_connector will set the client_is_connected  var to TRUE */            
      
      status = pthread_create(&thread_connect_id, NULL, TSP_streamer_sender_connector,  sock);
      TSP_CHECK_THREAD(status, FALSE);      

    }
  return sock;
}

void TSP_stream_sender_destroy(TSP_stream_sender_t sender)
{

   TSP_socket_t* sock = (TSP_socket_t*)sender;

   if( sock->fifo_size > 0)
     {
       RINGBUF_PTR_DESTROY(sock->out_ringbuf);
     }
   else
     {
         free(sock->out_item);sock->out_item = 0;
     }
   sock->out_ringbuf = NULL;
   free(sock);     

}

void 
TSP_stream_sender_stop(TSP_stream_sender_t sender) {

   TSP_socket_t* sock = (TSP_socket_t*)sender;
   
   sock->is_stopped = TRUE;

   shutdown(sock->socketId, SHUT_RDWR);
   shutdown(sock->hClient, SHUT_RDWR);
   close(sock->socketId);
   close(sock->hClient);

   /* Is there was a thread for the fifo, we must wait for it to end */
   if(sock->fifo_size > 0) {
     pthread_join(sock->thread_id, NULL);
   }
}

int 
TSP_stream_sender_send(TSP_stream_sender_t sender, const char *buffer, int bufferLen) {
  
  int Total;
  int nwrite;
  TSP_socket_t* sock = (TSP_socket_t*)sender;
  int identSocket = sock->hClient;

  Total = 0;
  if (identSocket > 0) {      
    while (bufferLen > 0) {
/* sous windows _errno est la variable globale en lecture seule */
#ifndef WIN32
      errno = 0;
#endif
      /* FIXME is it really an error to get 0 as nwrite? */
#ifdef _WIN32
	if( (nwrite = send(identSocket, &buffer[Total], bufferLen,0)) <= 0 ) {
    if( _errno == EINTR ) {
#else
	  if( (nwrite = write(identSocket,  &buffer[Total], bufferLen)) <= 0 ) {
	if( errno == EINTR ) {
#endif
	  /* The write might have been interrupted by a signal */
	  nwrite = 0;
	}
	else {		  
#ifdef _WIN32
	  STRACE_DEBUG("send failed with errno = %d : <%s>",WSAGetLastError(),strerror(errno));
#else
	  STRACE_ERROR("send failed with errno = %d : <%s>",errno,strerror(errno));
#endif
	  sock->connection_ok = FALSE;
	  return FALSE;
	}
      }      
      Total += nwrite;
      bufferLen -= nwrite;
    } /* end while buffer not written fully */ 
  } else {
    STRACE_ERROR("identSocket = %d",identSocket);
    return (FALSE);
  }

  return(TRUE);
} /* end of TSP_stream_sender_send */


/**
 * Tells is a client is connected to the socket.
 * If a client crashes this still returns TRUE
 * @return TRUE if the client was connected
 */
int 
TSP_stream_sender_is_client_connected(TSP_stream_sender_t sender) {
  TSP_socket_t* sock = (TSP_socket_t*)sender;

  return sock->client_is_connected;
}

int 
TSP_stream_sender_is_connection_ok(TSP_stream_sender_t sender) {
  TSP_socket_t* sock = (TSP_socket_t*)sender;
  
  return sock->connection_ok;
}


TSP_stream_sender_ringbuf_t* 
TSP_stream_sender_get_ringbuf(TSP_stream_sender_t sender) {
  TSP_socket_t* sock = (TSP_socket_t*)sender;
  
  return sock->out_ringbuf;
}

TSP_stream_sender_item_t* 
TSP_stream_sender_get_buffer(TSP_stream_sender_t sender) {
  TSP_socket_t* sock = (TSP_socket_t*)sender;

  return sock->out_item;
}
