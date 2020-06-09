/*
 * Define your service specification in this file and run rpcgen -MN minifyjpeg.x
 */

struct JPEG_IN {
 opaque buffer<> ;
};
struct JPEG_OUT {
 opaque buffer<> ;
};

program MINIFY_PROG { /* RPC service name */
  version MINIFY_VERS {
    JPEG_OUT MINIFY_JPEG_PROC(JPEG_IN) = 1; /* proc1 */
  } = 1; /* version1 */
} = 0x3f230001; /* service id */