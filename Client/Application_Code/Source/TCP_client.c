/* Include its own header file */
#include "TCP_client.h"

/* Rx and Tx time outs are used to ensure the sockets do not wait too long for
missing data. */
static const TickType_t xReceiveTimeOut = pdMS_TO_TICKS(4000);
static const TickType_t xSendTimeOut = pdMS_TO_TICKS(2000);


/*
1. Create client socket
2. Create a buffer to send the file. FTP??
3. Connect the socket to the remote server socket
4. Send data to the remote server socket
5. Shutdown the socket
6. Disconnect the socket
7. Close the socket
*/


Socket_t vCreateTCPClientSocket(struct freertos_sockaddr* xServerAddress)
{
    /* This function is to create a client socket and connect it to the server socket*/
    Socket_t xClientSocket;
    socklen_t xSize = sizeof(xServerAddress);
    static const TickType_t xTimeOut = pdMS_TO_TICKS(2000);

    /* Attempt to open the socket. */
    xClientSocket = FreeRTOS_socket(FREERTOS_AF_INET,
        FREERTOS_SOCK_STREAM,  /* SOCK_STREAM for TCP. */
        FREERTOS_IPPROTO_TCP);

    /* Check the socket was created. */
    configASSERT(xClientSocket != FREERTOS_INVALID_SOCKET);

    FreeRTOS_setsockopt(xClientSocket, 0, FREERTOS_SO_SNDTIMEO, &xTimeOut, sizeof(xTimeOut));

    FreeRTOS_bind(xClientSocket, NULL, xSize);


    WinProperties_t  xWinProps;
    xWinProps.lTxBufSize = 6 * ipconfigTCP_MSS;
    xWinProps.lTxWinSize = 6;
    xWinProps.lRxBufSize = 2 * ipconfigTCP_MSS;
    xWinProps.lRxWinSize = 1;
    FreeRTOS_setsockopt(xClientSocket, 0, FREERTOS_SO_WIN_PROPERTIES, (void*)&xWinProps, sizeof(xWinProps));


    //Connect the socket to the remote socket
    vConnect_Socket(xClientSocket, xServerAddress);
    return xClientSocket;
}

void vConnect_Socket(void* socket_address, struct freertos_sockaddr* xServerAddress)
{
    Socket_t xClientSocket = (Socket_t)socket_address;
    socklen_t xSize = sizeof(xServerAddress);
    int connection_counter = 1;
    if (FreeRTOS_issocketconnected(xClientSocket) != pdTRUE)
    {
        BaseType_t connection_status = FreeRTOS_connect(xClientSocket, xServerAddress, xSize);
        while (connection_status != 0) {
            FreeRTOS_printf(("Socket Connection Attempt Failed:%d, Error no: %d \n",
                connection_counter, connection_status));
            if (connection_status == -pdFREERTOS_ERRNO_EINPROGRESS)break;
            connection_counter++;
            connection_status = FreeRTOS_connect(xClientSocket, xServerAddress, xSize);

        }
        if (connection_counter < 11) {
            FreeRTOS_printf(("Socket connected to server. \n"));
        }
    }
    else
    {
        FreeRTOS_printf(("Socket is already in a connected state. \n"));
        vTaskDelay(500);
    }

}

int vSendMessage(void* socket_address, pixel_t* pcBufferToTransmitt, BaseType_t xTotalLengthToSend)
{
    /*This function is to send the data from the buffer to the server socket and close the
    client socket*/
    long start_ticker=0, send_tick_counter=0;
    FreeRTOS_printf(("Send message started. Message length: %d \n", xTotalLengthToSend));
    Socket_t xClientSocket = (Socket_t)socket_address;
    BaseType_t xAlreadyTransmitted = 0, xBytesSent = 0;
    BaseType_t xLenToSend;

    /* Keep sending until the entire buffer has been sent. */        
        while (xAlreadyTransmitted < xTotalLengthToSend)
        {
            start_ticker = xTaskGetTickCount();
            /* How many bytes are left to send? */
            //xLenToSend = (xTotalLengthToSend - xAlreadyTransmitted < ipconfigTCP_MSS) ? (xTotalLengthToSend - xAlreadyTransmitted) : ipconfigTCP_MSS;
            xLenToSend = xTotalLengthToSend - xAlreadyTransmitted;
            xBytesSent = FreeRTOS_send(xClientSocket, &(pcBufferToTransmitt[xAlreadyTransmitted]), xLenToSend, 0);
            if (xBytesSent >= 0)
            {
                /* Data was sent successfully. */
                xAlreadyTransmitted += xBytesSent;
                FreeRTOS_printf(("Bytes sent till now: %d \n", xAlreadyTransmitted));
            }
            else if (xBytesSent == -pdFREERTOS_ERRNO_ENOSPC)
            {
                //No space is left on the Tx buffer to send. So, try again.
                FreeRTOS_printf(("Send Error: %d \n", xBytesSent));
                continue;
            }
            else
            {
                FreeRTOS_printf(("Send Error: %d \n", xBytesSent));
                break;
            }
            
            send_tick_counter += xTaskGetTickCount() - start_ticker;
        }
    
        FreeRTOS_printf(("Total Bytes sent: %d \n", xAlreadyTransmitted));
    double extime = (send_tick_counter * 1.0) / configTICK_RATE_HZ;
    FreeRTOS_printf(("Execution time for data send= %f \n", extime));

    

    //Useful to close the socket automatically after the last packet of the file is sent
    /*BaseType_t xCloseAfterNextSend = pdTRUE;
    FreeRTOS_setsockopt(xClientSocket,
        0,
        FREERTOS_SO_CLOSE_AFTER_SEND,
        (void*)&xCloseAfterNextSend,
        sizeof(xCloseAfterNextSend));
        */
    
    //Shutdown the socket, i.e. read and write are blocked on it henceforth
    FreeRTOS_shutdown(xClientSocket, FREERTOS_SHUT_RDWR);

    //Wait for socket to disconnect before closing the socket.
    while (FreeRTOS_recv(xClientSocket, pcBufferToTransmitt, xTotalLengthToSend, 0) >= 0);

    //Safely close the socket
    FreeRTOS_closesocket(xClientSocket);
    return xBytesSent;
}


void vCloseSocket(void* socket_address, pixel_t* pcBufferToTransmitt, BaseType_t xTotalLengthToSend)
{
    Socket_t xClientSocket = (Socket_t)socket_address;
    
}