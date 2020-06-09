#include "magickminify.h"
#include "minifyjpeg.h"

/* Implement the server-side functions here */
bool_t minify_jpeg_proc_1_svc(JPEG_IN in, JPEG_OUT * out, struct svc_req * svcReq)
{
    bool_t ret = FALSE;
    void magickminify_init();

    out->buffer.buffer_val = (char*) magickminify(in.buffer.buffer_val, in.buffer.buffer_len, ((ssize_t*)&out->buffer.buffer_len));
    if (out->buffer.buffer_val != NULL)
    {
        ret = TRUE;
    }

    magickminify_cleanup();
    
    return ret;
}

int minify_prog_1_freeresult (SVCXPRT * prt, xdrproc_t proc, caddr_t addr)
{
    int ret = 1;

    return ret;
}