/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

 /******************************************************************************
  *Project description to be written here 
  */

  /* Standard includes. */
#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/*TCP includes*/
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"

/*Application includes*/
#include "_canny_.h"
#include "_rsa_.h"
#include "TCP_client.h"
#include "TCP_Server.h"

/* Define a name that will be used for LLMNR and NBNS searches. */
#define mainHOST_NAME				"Runway_Detection_client"
#define mainDEVICE_NICK_NAME		"windows_demo_client"

/* heap_5.c is used. Need to check out heap_4.c*/
#define mainREGION_1_SIZE	7201
#define mainREGION_2_SIZE	29905
#define mainREGION_3_SIZE	6407

/* Set the following constant to pdTRUE to log using the method indicated by the
name of the constant, or pdFALSE to not log using the method indicated by the
name of the constant.  Options include to standard out (xLogToStdout), to a disk
file (xLogToFile), and to a UDP port (xLogToUDP).  If xLogToUDP is set to pdTRUE
then UDP messages are sent to the IP address configured as the echo server
address (see the configECHO_SERVER_ADDR0 definitions in FreeRTOSConfig.h) and
the port number set by configPRINT_PORT in FreeRTOSConfig.h. */
const BaseType_t xLogToStdout = pdTRUE, xLogToFile = pdFALSE, xLogToUDP = pdFALSE;


/*
 * Prototypes for the standard FreeRTOS application hook (callback) functions
 * implemented within this file.  See http://www.freertos.org/a00016.html .
 */
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName);
void vApplicationTickHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer, uint32_t* pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer, StackType_t** ppxTimerTaskStackBuffer, uint32_t* pulTimerTaskStackSize);

/*-----------------------------------------------------------*/

/*
 * Prototypes for the TCP utility functions
 */
UBaseType_t uxRand(void);
static void prvSRand(UBaseType_t ulSeed);

/*
* Prototypes for the application code
*/
void initialise_network();
Socket_t initialise_client_socket(void);
void vRunApplication(void);
void vLoadImage(void);
void CannyFilter();
void RSAencryption();
void vTCPSend(void);

/* Use by the pseudo random number generator. */
static UBaseType_t ulNextRand;
Socket_t client_socket;
size_t server_flag = 0, client_flag=0;//semaphores to be used here instead of global variables

/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
use a callback function to optionally provide the memory required by the idle
and timer tasks.  This is the stack that will be used by the timer task.  It is
declared here, as a global, so it can be checked by a test that is implemented
in a different file. */
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
/*-----------------------------------------------------------*/

//Variable to count the number of ticks to calculate the time consumption
BaseType_t my_ticker = 0;

//queue handle
QueueHandle_t my_queue_handle;


/*
* Struct to send and  recieve image data in queue
*/
typedef struct my_image
{
	bitmap_info_header_t ih;
	const pixel_t* bitmap_data;
	int my_image_len;

} myIMAGE_DATA;

//To store the loaded image data
myIMAGE_DATA loaded_data;

void initialise_network()
{
	/* Miscellaneous initialisation including preparing the logging and seeding
	the random number generator. */

	time_t* xTimeNow = (time_t*)malloc(sizeof(time_t));
	uint32_t ulLoggingIPAddress;

	ulLoggingIPAddress = FreeRTOS_inet_addr_quick(192, 168, 190, 129);
	vLoggingInit(xLogToStdout, xLogToFile, xLogToUDP, ulLoggingIPAddress, configPRINT_PORT);

	/* Seed the random number generator. */
	time(xTimeNow);
	FreeRTOS_debug_printf(("Seed for randomiser: %lu\n", *xTimeNow));
	prvSRand((uint32_t)(*xTimeNow));
	FreeRTOS_debug_printf(("Random numbers: %08X %08X %08X %08X\n", ipconfigRAND32(), ipconfigRAND32(), ipconfigRAND32(), ipconfigRAND32()));

	/* Initialising the IP Stack*/
	FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
}

Socket_t initialise_client_socket(void)
{
	//This is a task function with a priority of 8 started by the vRunApplication
	
	//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	struct freertos_sockaddr xRemoteAddress;
	/* Set the IP address (192.168.xx.xx) and port (7) of the remote socket
	to which this client socket will transmit. */
	xRemoteAddress.sin_port = FreeRTOS_htons(7);
	xRemoteAddress.sin_addr = FreeRTOS_inet_addr_quick(192, 168, 190, 149);
	client_socket = vCreateTCPClientSocket(&xRemoteAddress);

	vTaskPrioritySet(NULL, 2);
	xTaskNotifyGive(xTaskGetHandle("Run"));
	//client_flag = 1;
	for (;;);
}

void vLoadImage(void)
{
	my_queue_handle = xQueueCreate(1, sizeof(myIMAGE_DATA));
	bitmap_info_header_t ih;
	const pixel_t* in_bitmap_data = load_bmp(".\\inputs\\_lenna.bmp", &ih);
	configASSERT(in_bitmap_data != NULL);
	loaded_data.bitmap_data = in_bitmap_data;
	loaded_data.ih = ih;
	loaded_data.my_image_len = ih.height * ih.width;
	FreeRTOS_printf(("Load Image is done. \n"));
	//vTaskPrioritySet(NULL, 2);
}

void vRunApplication(void)
{
	//This is a task function with a priority of 3 started by the NetworkEventHook

	xTaskHandle dummy_handle;
	xTaskCreate(initialise_client_socket, "Client", 1000, NULL, 5, &dummy_handle);
	
	//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	for (;;) 
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		BaseType_t send_result = xQueueSend(my_queue_handle, &loaded_data, 0);
		while (send_result != pdTRUE);
		
		TaskHandle_t canny_handle, encryption_handle, send_handle;
		xTaskCreate(CannyFilter, "Canny", 1000, NULL, 7, &canny_handle);
		xTaskCreate(RSAencryption, "encrypt", 1000, NULL, 6, &encryption_handle);
		xTaskCreate(vTCPSend, "send", 1000, NULL, 5, &send_handle);
		xTaskNotifyGive(xTaskGetHandle("Canny"));
		//xTaskNotifyGive(dummy_handle);
	}
}

void CannyFilter()
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	myIMAGE_DATA loaded_data_local; // Variable to recieve/process/send data from/to queue
	xQueueReceive(my_queue_handle, &loaded_data_local, 0);
	pixel_t* in_bitmap_data = loaded_data_local.bitmap_data;
	bitmap_info_header_t ih = loaded_data_local.ih;

	my_ticker = 0;
	pixel_t* out_bitmap_data =
		canny_edge_detection(in_bitmap_data, &ih, 40, 80, 1.0);
	configASSERT(out_bitmap_data != NULL);

	loaded_data_local.bitmap_data = out_bitmap_data;
	loaded_data_local.ih = ih;
	BaseType_t send_resut= xQueueSend(my_queue_handle, &loaded_data_local, 0);
	while (send_resut != pdTRUE);
	
	xTaskNotifyGive(xTaskGetHandle("encrypt"));
	vTaskDelete(NULL);
}

void RSAencryption()
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	myIMAGE_DATA loaded_data_local;
	xQueueReceive(my_queue_handle, &loaded_data_local, 0);
	pixel_t* edges_data = loaded_data_local.bitmap_data;
	bitmap_info_header_t ih = loaded_data_local.ih;
	int msg_length = ih.height * ih.width;
	compressed_image en_img = rsa_encryption(edges_data, msg_length);
	FreeRTOS_printf(("Encryption is complete \n"));
	vTaskPrioritySet(NULL, 6);

	configASSERT(edges_data != NULL);
	loaded_data_local.bitmap_data = en_img.en_msg;
	loaded_data_local.my_image_len = en_img.en_msg_length;
	FreeRTOS_printf(("Length of compressed image= %d, last value=%d \n", en_img.en_msg_length, en_img.en_msg[en_img.en_msg_length-1]));
	BaseType_t send_result = xQueueSend(my_queue_handle, &loaded_data_local, 0);
	while (send_result != pdTRUE);
	
	xTaskNotifyGive(xTaskGetHandle("send"));
	vTaskDelete(NULL);
}

void vTCPSend(void)
{
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	myIMAGE_DATA loaded_data_local;
	xQueueReceive(my_queue_handle, &loaded_data_local, 0);
	pixel_t* encrypted_data = loaded_data_local.bitmap_data;

	int msg_length = loaded_data.my_image_len;
	printf("encrypted data: %lu \n", encrypted_data[msg_length-1]);
 	int send_result= vSendMessage(client_socket, encrypted_data, msg_length);
	while (send_result == 0);

	//vTaskDelete(xTaskGetHandle("Client"));

	xTaskNotifyGive(xTaskGetHandle("Run"));
	
	vTaskDelete(NULL);

	for (;;);
}

int main(void)
{
	//prvInitialiseHeap();
	initialise_network();
	//my_semaphore_handle = xSemaphoreCreateBinary();
	vTaskStartScheduler();
	for (;;);
	return 0;
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	 */
	//printf("pvPortMalloc has failed \n");
	vAssertCalled(__LINE__, __FILE__);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char* pcTaskName)
{
	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.  This function is
	provided as an example only as stack overflow checking does not function
	when running the FreeRTOS Windows port. */
	printf("There is a stack overflow \n");
	vAssertCalled(__LINE__, __FILE__);
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void)
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
	TaskHandle_t canny_handle = xTaskGetHandle("canny");
	if (canny_handle != NULL)
		if(eTaskGetState(canny_handle)== eRunning) my_ticker++;

}
/*-----------------------------------------------------------*/

void vApplicationIPNetworkEventHook(eIPCallbackEvent_t eNetworkEvent)
{
	uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
	char cBuffer[16];
	static BaseType_t xTasksAlreadyCreated = pdFALSE;

	/* Both eNetworkUp and eNetworkDown events can be processed here. */
	if (eNetworkEvent == eNetworkUp)
	{
		/* Create the tasks that use the TCP/IP stack if they have not already
		been created. */
		if (xTasksAlreadyCreated == pdFALSE)
		{
			/*
			 * For convenience, tasks that use FreeRTOS+TCP can be created here
			 * to ensure they are not created before the network is usable.
			 */
			xTaskHandle Run_application_handle;
			
			vLoadImage();
			xTaskCreate(vRunApplication, "Run", 1000, NULL, 3, &Run_application_handle);
			
			server_flag = 1;// Denotes that the client and the server are up and running
			xTasksAlreadyCreated = pdTRUE;
		}
		FreeRTOS_GetAddressConfiguration(&ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress);
		FreeRTOS_inet_ntoa(ulIPAddress, cBuffer);
		FreeRTOS_printf(("\r\n\r\nIP Address: %s\r\n", cBuffer));
	}
}
/*-------------------------------------------------------------*/
void vAssertCalled(unsigned long ulLine, const char* const pcFileName)
{
	volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

	/* Called if an assertion passed to configASSERT() fails.  See
	http://www.freertos.org/a00110.html#configASSERT for more information. */

	/* Parameters are not used. */
	printf("Line number= %d \n", ulLine);
	printf("Fine name= %s \n", pcFileName);
	(void)ulLine;
	(void)pcFileName;

	printf("ASSERT! Line %ld, file %s, GetLastError() %ld\r\n", ulLine, pcFileName, GetLastError());

	taskENTER_CRITICAL();
	{

		/* You can step out of this function to debug the assertion by using
		the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
		value. */
		while (ulSetToNonZeroInDebuggerToContinue == 0)
		{
			__asm { NOP };
			__asm { NOP };
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/*static void  prvInitialiseHeap(void)
{
	The Windows demo could create one large heap region, in which case it would
	be appropriate to use heap_4.  However, purely for demonstration purposes,
	heap_5 is used instead, so start by defining some heap regions.  No
	initialisation is required when any other heap implementation is used.  See
	http://www.freertos.org/a00111.html for more information.

	The xHeapRegions structure requires the regions to be defined in start address
	order, so this just creates one big array, then populates the structure with
	offsets into the array - with gaps in between and messy alignment just for test
	purposes. 
	static uint8_t ucHeap[configTOTAL_HEAP_SIZE];
	volatile uint32_t ulAdditionalOffset = 19; Just to prevent 'condition is always true' warnings in configASSERT(). 
	const HeapRegion_t xHeapRegions[] =
	{
		// Start address with dummy offsets						Size 
		{ ucHeap + 1,											mainREGION_1_SIZE },
		{ ucHeap + 15 + mainREGION_1_SIZE,						mainREGION_2_SIZE },
		{ ucHeap + 19 + mainREGION_1_SIZE + mainREGION_2_SIZE,	mainREGION_3_SIZE },
		{ NULL, 0 }
	};

	// Sanity check that the sizes and offsets defined actually fit into the
	//array. 
	configASSERT((ulAdditionalOffset + mainREGION_1_SIZE + mainREGION_2_SIZE + mainREGION_3_SIZE) < configTOTAL_HEAP_SIZE);

	// Prevent compiler warnings when configASSERT() is not defined.
	(void)ulAdditionalOffset;

	vPortDefineHeapRegions(xHeapRegions);
}*/
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer, StackType_t** ppxIdleTaskStackBuffer, uint32_t* pulIdleTaskStackSize)
{
	/* If the buffers to be provided to the Idle task are declared inside this
	function then they must be declared static - otherwise they will be allocated on
	the stack and so not exists after this function exits. */
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t** ppxTimerTaskTCBBuffer, StackType_t** ppxTimerTaskStackBuffer, uint32_t* pulTimerTaskStackSize)
{
	/* If the buffers to be provided to the Timer task are declared inside this
	function then they must be declared static - otherwise they will be allocated on
	the stack and so not exists after this function exits. */
	static StaticTask_t xTimerTaskTCB;

	/* Pass out a pointer to the StaticTask_t structure in which the Timer
	task's state will be stored. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

	/* Pass out the array that will be used as the Timer task's stack. */
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;

	/* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
	Note that, as the array is necessarily of type StackType_t,
	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

UBaseType_t uxRand(void)
{
	const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = (ulMultiplier * ulNextRand) + ulIncrement;
	return((int)(ulNextRand >> 16UL) & 0x7fffUL);
}
/*-----------------------------------------------------------*/

static void prvSRand(UBaseType_t ulSeed)
{
	/* Utility function to seed the pseudo random number generator. */
	ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/

#if( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 )

BaseType_t xApplicationDNSQueryHook(const char* pcName)
{
	BaseType_t xReturn;

	/* Determine if a name lookup is for this node.  Two names are given
	to this node: that returned by pcApplicationHostnameHook() and that set
	by mainDEVICE_NICK_NAME. */
	if (_stricmp(pcName, mainHOST_NAME) == 0)
	{
		xReturn = pdPASS;
	}
	else if (_stricmp(pcName, mainDEVICE_NICK_NAME) == 0)
	{
		xReturn = pdPASS;
	}
	else
	{
		xReturn = pdFAIL;
	}

	return xReturn;
}

#endif

#if( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) || ( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
const char* pcApplicationHostnameHook(void)
{
	/* Assign the name "FreeRTOS" to this network node.  This function will
	be called during the DHCP: the machine will be registered with an IP
	address plus this name. */
	return mainHOST_NAME;
}
#endif
/*-----------------------------------------------------------*/

/*
 * Callback that provides the inputs necessary to generate a randomized TCP
 * Initial Sequence Number per RFC 6528.  THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 */
extern uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress,
	uint16_t usSourcePort,
	uint32_t ulDestinationAddress,
	uint16_t usDestinationPort)
{
	(void)ulSourceAddress;
	(void)usSourcePort;
	(void)ulDestinationAddress;
	(void)usDestinationPort;

	return uxRand();
}

/*
 * Supply a random number to FreeRTOS+TCP stack.
 * THIS IS ONLY A DUMMY IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM NUMBER
 * SO IS NOT INTENDED FOR USE IN PRODUCTION SYSTEMS.
 */
BaseType_t xApplicationGetRandomNumber(uint32_t* pulNumber)
{
	*(pulNumber) = uxRand();
	return pdTRUE;
}
