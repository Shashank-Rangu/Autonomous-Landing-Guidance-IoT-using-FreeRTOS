#include "TCP_Server.h"
/* Application includes*/
#include "_canny_.h"

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

void Landing_guidance(void* );

void xRecieveTCPData(Socket_t xConnectedSocket)
{
    long server_tick_counter = 0;
    long start_ticker;
    int32_t lBytes;
    static const TickType_t xReceiveTimeOut = pdMS_TO_TICKS(5000);
    static const TickType_t xSendTimeOut = pdMS_TO_TICKS(5000);
    pixel_t* pucRxBuffer= (pixel_t*)malloc(20000 * sizeof(pixel_t));
    
    memset(pucRxBuffer, 0x00, 20000 * sizeof(pixel_t));

    BaseType_t TotalBytes = 6000, TotalRecieved = 0;
    while (TotalRecieved< TotalBytes)
    {
        //start_ticker = xTaskGetTickCount();
        lBytes = FreeRTOS_recv(xConnectedSocket, &(pucRxBuffer[TotalRecieved]), ipconfigTCP_MSS, 0);
        if (lBytes >= 0)
        {
            TotalRecieved += lBytes;
            FreeRTOS_printf(("Total recieved till now: %d\n", TotalRecieved));
        }
        else
            FreeRTOS_printf(("recieved: %d \n", lBytes));
        //server_tick_counter += xTaskGetTickCount() - start_ticker;
    }
    FreeRTOS_printf(("Total recieved output: %d\n", TotalRecieved));

    //Shutdown the socket, i.e. read and write are blocked on it henceforth
    FreeRTOS_shutdown(xConnectedSocket, FREERTOS_SHUT_RDWR);

    //Safely close the socket
    FreeRTOS_closesocket(xConnectedSocket);
    //long stop_ticker = xTaskGetTickCount() - start_ticker;
    //double extime = (server_tick_counter * 1.0) / configTICK_RATE_HZ;
    //FreeRTOS_printf(("Execution time of server recieve= %f \n", extime));
}


Socket_t* vCreateTCPServerSocket(void)
{
    //printf("Started server_socket_create\n");
    Socket_t xServerSocket;
    struct freertos_sockaddr xBindAddress;
    socklen_t xSize = sizeof(xBindAddress);
    static const TickType_t xReceiveTimeOut = portMAX_DELAY;
    const BaseType_t xBacklog = 20; //Limiting the number of clients to 20

    /* Attempt to open the TCP socket. */
    xServerSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

    /* Check the socket was created. */
    configASSERT(xServerSocket != FREERTOS_INVALID_SOCKET);

    /*Set a time out so accept() will just wait for a connection.*/
    FreeRTOS_setsockopt(xServerSocket,0,FREERTOS_SO_RCVTIMEO,&xReceiveTimeOut,sizeof(xReceiveTimeOut));

    /* Set the listening port to 15000. */
    //xBindAddress.sin_port = (uint16_t)7;
    xBindAddress.sin_port = FreeRTOS_htons(7);

    //Bind the socket to the port
    configASSERT(!FreeRTOS_bind(xServerSocket, &xBindAddress, xSize));

    ///////Put the port in listening state
    BaseType_t socket_state = FreeRTOS_listen(xServerSocket, xBacklog);
    configASSERT(FreeRTOS_listen(xServerSocket, xBacklog));
    //printf("Started server_listen\n");
    Socket_t xConnectedSocket;
    struct freertos_sockaddr xClient;
    //socklen_t xSize = sizeof(xClient);
    for (;; )
    {
        // Wait for incoming connections.
        xConnectedSocket = FreeRTOS_accept(xServerSocket, &xClient, &xSize);
        configASSERT(xConnectedSocket != FREERTOS_INVALID_SOCKET);
        FreeRTOS_printf(("Socket connected to server \n"));

        
        xTaskCreate(xRecieveTCPData, "Recieve Data", 1000, xConnectedSocket, 8, NULL);
        xTaskCreate(Landing_guidance, "Landing guidance", configMINIMAL_STACK_SIZE, (void*)NULL, tskIDLE_PRIORITY, NULL);

    }
    return xServerSocket;
}


void Landing_guidance(void* pvParameters)
{
    /*Here we are supposed to start an image processing task for guiding the aeroplane
    for landing. Since this is not the scope of our assignment, a dummy delay is implemented*/
    vTaskDelay(250);
    //Assuming that the server is very fast, a small simulated delay is put.
    return NULL;
}