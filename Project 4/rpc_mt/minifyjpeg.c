#include "magickminify.h"
#include "minifyjpeg.h"


#include <pthread.h>
#include <fcntl.h>
#include <errno.h>

/* Implement the server-side functions here */
bool_t minify_jpeg_proc_1_svc(JPEG_IN in, xdrproc_t *xdrArg, struct svc_req *svcReq, SVCXPRT *transp)
{
    bool_t ret = false;

	minifyRequest_t *request = (minifyRequest_t*) malloc(sizeof(minifyRequest_t));
    request->svcReq = svcReq;    
    request->jpeg_in_arg = in;
    request->xdr_argument = xdrArg;
    request->transp = transp;

    if (requestQueue)
    {
        pthread_mutex_lock(&g_locks->mutex);
        steque_enqueue(requestQueue, request);
        pthread_mutex_unlock(&g_locks->mutex);
    }

    if (pthread_cond_broadcast(&g_locks->cond) != 0)
    {
        printf("Function %s Line: %d Broadcast Failed with Error %d ! \n", __FUNCTION__, __LINE__, errno);
    }
    #if DEBUG_LOG		
    else
    {
        printf("Function %s Line: %d Broadcast success with Error %d ! \n", __FUNCTION__, __LINE__, errno);
    }
    #endif //DEBUG_LOG
    
    return ret;
}

int minify_prog_1_freeresult (SVCXPRT * prt, xdrproc_t proc, caddr_t addr)
{
    int ret = 1;

    return ret;
}