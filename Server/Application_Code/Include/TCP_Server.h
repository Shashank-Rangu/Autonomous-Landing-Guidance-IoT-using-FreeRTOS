/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"


/* TCP Server functions*/
Socket_t* vCreateTCPServerSocket(void);
