/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <memory.h> /* for memset */
#include "tsp_rpc.h"

int tsp_wrap_rpc_clnt_set_timeout(CLIENT *client, int timeout)
{
 static struct timeval tv;

 /* RPC timeout definition */
 /* ----------------------- */
 tv.tv_sec = timeout;
 tv.tv_usec = 0;
 return clnt_control(client, CLSET_TIMEOUT, (char *)&tv);
}
#define TSP_STRACE_RPC_ERROR(cl, pResult) if(!pResult) { STRACE_ERROR(("%s", clnt_sperror(cl, ""))); }

/* Default timeout can be changed using clnt_control() */
static struct timeval TIMEOUT = { 25, 0 };

TSP_provider_info_t *
tsp_provider_information_1(CLIENT *clnt)
{
	static TSP_provider_info_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	 if (clnt_call (clnt, TSP_PROVIDER_INFORMATION, (xdrproc_t) xdr_void, (caddr_t) NULL,
		(xdrproc_t) xdr_TSP_provider_info_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_open_t *
tsp_request_open_1(TSP_request_open_t req_open,  CLIENT *clnt)
{
	static TSP_answer_open_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_OPEN,
		(xdrproc_t) xdr_TSP_request_open_t, (caddr_t) &req_open,
		(xdrproc_t) xdr_TSP_answer_open_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
tsp_request_close_1(TSP_request_close_t req_close,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_CLOSE,
		(xdrproc_t) xdr_TSP_request_close_t, (caddr_t) &req_close,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_sample_t *
tsp_request_information_1(TSP_request_information_t req_info,  CLIENT *clnt)
{
	static TSP_answer_sample_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_INFORMATION,
		(xdrproc_t) xdr_TSP_request_information_t, (caddr_t) &req_info,
		(xdrproc_t) xdr_TSP_answer_sample_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_feature_t *
tsp_request_feature_1(TSP_request_feature_t req_feature,  CLIENT *clnt)
{
	static TSP_answer_feature_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_FEATURE,
		(xdrproc_t) xdr_TSP_request_feature_t, (caddr_t) &req_feature,
		(xdrproc_t) xdr_TSP_answer_feature_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_sample_t *
tsp_request_sample_1(TSP_request_sample_t req_sample,  CLIENT *clnt)
{
	static TSP_answer_sample_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_SAMPLE,
		(xdrproc_t) xdr_TSP_request_sample_t, (caddr_t) &req_sample,
		(xdrproc_t) xdr_TSP_answer_sample_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_sample_init_t *
tsp_request_sample_init_1(TSP_request_sample_init_t req_sample,  CLIENT *clnt)
{
	static TSP_answer_sample_init_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_SAMPLE_INIT,
		(xdrproc_t) xdr_TSP_request_sample_init_t, (caddr_t) &req_sample,
		(xdrproc_t) xdr_TSP_answer_sample_init_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_sample_destroy_t *
tsp_request_sample_destroy_1(TSP_request_sample_destroy_t req_destroy,  CLIENT *clnt)
{
	static TSP_answer_sample_destroy_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_SAMPLE_DESTROY,
		(xdrproc_t) xdr_TSP_request_sample_destroy_t, (caddr_t) &req_destroy,
		(xdrproc_t) xdr_TSP_answer_sample_destroy_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
tsp_exec_feature_1(TSP_exec_feature_t exec_feature,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_EXEC_FEATURE,
		(xdrproc_t) xdr_TSP_exec_feature_t, (caddr_t) &exec_feature,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

int *
tsp_request_async_sample_write_1(TSP_async_sample_t async_sample_write,  CLIENT *clnt)
{
	static int clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_ASYNC_SAMPLE_WRITE,
		(xdrproc_t) xdr_TSP_async_sample_t, (caddr_t) &async_sample_write,
		(xdrproc_t) xdr_int, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_async_sample_t *
tsp_request_async_sample_read_1(TSP_async_sample_t async_sample_read,  CLIENT *clnt)
{
	static TSP_async_sample_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_ASYNC_SAMPLE_READ,
		(xdrproc_t) xdr_TSP_async_sample_t, (caddr_t) &async_sample_read,
		(xdrproc_t) xdr_TSP_async_sample_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_sample_t *
tsp_request_filtered_information_1(TSP_request_information_t req_info, int filter_kind, char *filter_string,  CLIENT *clnt)
{
	tsp_request_filtered_information_1_argument arg;
	static TSP_answer_sample_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	arg.req_info = req_info;
	arg.filter_kind = filter_kind;
	arg.filter_string = filter_string;
	if (clnt_call (clnt, TSP_REQUEST_FILTERED_INFORMATION, (xdrproc_t) xdr_tsp_request_filtered_information_1_argument, (caddr_t) &arg,
		(xdrproc_t) xdr_TSP_answer_sample_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

TSP_answer_extended_information_t *
tsp_request_extended_information_1(TSP_request_extended_information_t req_extinfo,  CLIENT *clnt)
{
	static TSP_answer_extended_information_t clnt_res;

	memset((char *)&clnt_res, 0, sizeof(clnt_res));
	if (clnt_call (clnt, TSP_REQUEST_EXTENDED_INFORMATION,
		(xdrproc_t) xdr_TSP_request_extended_information_t, (caddr_t) &req_extinfo,
		(xdrproc_t) xdr_TSP_answer_extended_information_t, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}