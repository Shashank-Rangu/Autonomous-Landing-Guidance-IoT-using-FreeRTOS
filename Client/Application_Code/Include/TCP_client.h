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

#include "_canny_.h"

Socket_t vCreateTCPClientSocket(struct freertos_sockaddr* xServerAddress);
int vSendMessage(void *socket_address, short *pcBufferToTransmitt, BaseType_t xTotalLengthToSend);
void vConnect_Socket(void* socket_address, struct freertos_sockaddr* xServerAddress);
void vCloseSocket(void* socket_address, pixel_t* pcBufferToTransmitt, BaseType_t xTotalLengthToSend);
