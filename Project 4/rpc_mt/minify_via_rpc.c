#include "minifyjpeg_clnt.c"
#include "minifyjpeg_xdr.c"
#include <stdio.h>
#include <stdlib.h>

CLIENT* get_minify_client(char *server)
{
    CLIENT *cl;

    /* Your code goes here */
    struct timeval timeOutVal;

    /* Your code goes here */
    cl = clnt_create(server, MINIFY_PROG, MINIFY_VERS, "tcp");
    if(cl == NULL)
    {
        perror("clnt_create Failed ");
    }
    
    timeOutVal.tv_sec = 5000;
    timeOutVal.tv_usec = 0;

    clnt_control(cl, CLSET_TIMEOUT, (char*) &timeOutVal);

    return cl;
}


int minify_via_rpc(CLIENT *cl, void* src_val, size_t src_len, size_t *dst_len, void **dst_val)
{
	/*Your code goes here */
    int retCode;

    if (cl == NULL)
    {
        perror("CLIENT Hanlde is NULL, returning now ");
        return 0;
    }

    if (src_val == NULL)
    {
        perror("src_val is NULL, returning now ");
        return 0;
    }

    if (src_len <=0)
    {
        perror("src_len <=0, returning now ");
        return 0;
    }

    JPEG_IN srcJpeg ;
    srcJpeg.buffer.buffer_len = src_len;
    srcJpeg.buffer.buffer_val = src_val;

    JPEG_OUT dstJpeg;
    dstJpeg.buffer.buffer_val = NULL;
    dstJpeg.buffer.buffer_len = src_len/2+1;

    retCode = minify_jpeg_proc_1(srcJpeg, &dstJpeg, cl);

    if (dstJpeg.buffer.buffer_val != NULL)
    {
        *dst_len = dstJpeg.buffer.buffer_len;
        *dst_val = dstJpeg.buffer.buffer_val;
    }
    
    clnt_perrno(retCode); printf("\nretCode returned %d \n", retCode);

    return retCode;
}
