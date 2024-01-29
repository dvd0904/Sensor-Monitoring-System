#ifndef OS_HEADERS
#define OS_HEADERS


/* Size limit control */
#define OS_SIZE_65536               65536
#define OS_SIZE_2048                2048
#define OS_SIZE_1024                1024
#define OS_SIZE_512                 512
#define OS_SIZE_256                 256
#define OS_SIZE_128                 128
#define OS_SIZE_64                  64
#define OS_SIZE_32                  32
#define OS_SIZE_16                  16


/* Level of log messages */
#define LOGLEVEL_CRITICAL           4
#define LOGLEVEL_ERROR              3
#define LOGLEVEL_WARNING            2
#define LOGLEVEL_INFO               1
#define LOGLEVEL_DEBUG              0


#define OS_MAXSTR                   OS_SIZE_65536       
#define OS_BUFFER_SIZE              OS_SIZE_512        
#define OS_MSG_SIZE                 OS_SIZE_128
#define IPSIZE                      INET6_ADDRSTRLEN    /* IP Address size                  */
#define THREADS_NUM                 3
#define BACKLOG                     128                 /* Socket input queue length        */
#define MIN_PORT                    1024                /* Minimum port number              */
#define MAX_PORT                    65536               /* Maximum port number              */
#define SLEEP                       3
#define MIN_TEMP                    17
#define MAX_TEMP                    35
#define READER_THREADS              2                   /* Number of thread can read shared data */

#define __ass_name                  "Final Assignment"
#define __author                    "Nguyen Dinh Dai"
#define __contact                   "dinhdai0904@gmail.com"


#endif /* OS_HEADERS */