#include "TCP_Server.h"
/* Application includes*/
#include "_canny_.h"

/* Rx time out are used to ensure the sockets do not wait too long for
missing data. */
static const TickType_t xReceiveTimeOut = pdMS_TO_TICKS(4000);

/*
1. Create server socket
2. Create a buffer to recieve the file.
3. Bind the socket to a port
4. Set the socket in listening state and accept incoming connections
5. Recieve the data and store in the buffer
6. Shutdown the socket
7. Close the socket
8. Landing guidance module
*/

void Landing_guidance(void*);

void xRecieveTCPData(Socket_t xConnectedSocket)
{
    long server_tick_counter = 0;
    long start_ticker;
    BaseType_t lBytes;//To count the number of recieved in each iteration
    pixel_t* pucRxBuffer = (pixel_t*)malloc(300000 * sizeof(pixel_t));

    BaseType_t TotalRecieved = 0;//To print the number of bytes recieved
    while (1)
    {
        lBytes = FreeRTOS_recv(xConnectedSocket, &(pucRxBuffer[TotalRecieved/2]), ipconfigTCP_MSS, 0);
        if (lBytes >= 0)
        {
            TotalRecieved += lBytes;
            if (pucRxBuffer[TotalRecieved/2 - 1] == 5000)//end of file reached
            {
                xTaskCreate(Landing_guidance, "Landing guidance", configMINIMAL_STACK_SIZE, (void*)NULL, tskIDLE_PRIORITY, NULL);
                FreeRTOS_printf(("Total recieved output: %d\n", TotalRecieved/2));
                TotalRecieved = 0;
            }
        }
        else if(lBytes == 0) continue;
        else if(TotalRecieved != 0)
        {
            xTaskCreate(Landing_guidance, "Landing guidance", configMINIMAL_STACK_SIZE, (void*)NULL, tskIDLE_PRIORITY, NULL);
            FreeRTOS_printf(("Total recieved output: %d\n", TotalRecieved));
            TotalRecieved = 0;
        }
        
        
    }
    

    free(pucRxBuffer);

    //Shutdown the socket, i.e. read and write are blocked on it henceforth
    FreeRTOS_shutdown(xConnectedSocket, FREERTOS_SHUT_RDWR);

    //Safely close the socket
    FreeRTOS_closesocket(xConnectedSocket);    
    vTaskDelete(NULL);
}


Socket_t* vCreateTCPServerSocket(void)
{
    Socket_t xServerSocket;
    struct freertos_sockaddr xBindAddress;
    socklen_t xSize = sizeof(xBindAddress);
    static const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const BaseType_t xBacklog = 20; //Limiting the number of clients to 20

    /* Attempt to open the TCP socket. */
    xServerSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

    /* Check the socket was created. */
    configASSERT(xServerSocket != FREERTOS_INVALID_SOCKET);

    /*Set the sliding window properties of the TCP Server*/
    WinProperties_t xWindow;
    xWindow.lRxBufSize = 4*ipconfigTCP_MSS;
    xWindow.lRxWinSize = 2;
    xWindow.lTxBufSize = 1 * ipconfigTCP_MSS;
    xWindow.lTxWinSize = 1;
    FreeRTOS_setsockopt(xServerSocket, 0, FREERTOS_SO_WIN_PROPERTIES, (void*)&xWindow, sizeof(xWindow));


    /*Set a time out so accept() will just wait for a connection.*/
    FreeRTOS_setsockopt(xServerSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof(xReceiveTimeOut));

    /* Set the listening port to 7. */
    xBindAddress.sin_port = FreeRTOS_htons(7);

    //Bind the socket to the port
    configASSERT(!FreeRTOS_bind(xServerSocket, &xBindAddress, xSize));

    ///////Put the port in listening state
    BaseType_t socket_state = FreeRTOS_listen(xServerSocket, xBacklog);
    configASSERT(FreeRTOS_listen(xServerSocket, xBacklog));

    Socket_t xConnectedSocket;
    struct freertos_sockaddr xClient;
    for (;; )
    {
        // Wait for incoming connections.
        xConnectedSocket = FreeRTOS_accept(xServerSocket, &xClient, &xSize);
        configASSERT(xConnectedSocket != FREERTOS_INVALID_SOCKET);
        FreeRTOS_printf(("Socket connected to server \n"));
        xTaskCreate(xRecieveTCPData, "Receive Data", 1000, xConnectedSocket, 3, NULL);
    }
    return xServerSocket;
}


void Landing_guidance(void* pvParameters)
{
    /*Here we are supposed to start an image processing task for guiding the aeroplane
    for landing. Since this is not the scope of our assignment, a dummy delay is implemented*/
    vTaskDelay(250);
    //Assuming that the server is very fast, a small simulated delay is put.
    vTaskDelete(NULL);
    return NULL;
}