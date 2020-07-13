# How to use this code? 

**OS used:** Windows 10
**Prerequisites:** Basic knowledge of FreeRTOS, Visual Studio installed- This was developed and tested using Visual Studio Community, and a LAN connected PC.

1. Open the 'Server' folder and open the visual studio project file. 
2. Build and run this project. *This simulates the server side of the code which would supposedly be in the ground station.*
    * The FreeRTOS + TCP part of the code connects to the network. This can be found on the official FreeRTOS website as well.
    * This assigns a static IP address which is hard coded. *You can change this value or can change to the code to optain a dynamic IP address*
    *  A listening TCP socket is created which is bound to fixed port and is ready to accept new connections.
3. Open the 'Client' folder and open the visual studio project file. 
4. Build and run this project after the server code is up and running. 
    * The FreeRTOS + TCP part of the code connects to the network. Similar to the server side. 
    * This will create the client side and connect to the hard coded server IP address and port number. *You need to modify the code if you change it above*
5. Once both server side and client side are up and running, you can see that the client has connected to the server, and is processing and transmitting the data. 

The details of each of the FreeRTOS tasks, priorities, configuration and the tested outputs are given the report: 'Autonomous Runway Detection report.docx'
