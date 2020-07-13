# Autonomous Runway Dection and Landing Guidance using FreeRTOS
Objective of this project is to develop a mechanism to autonomously guide an aeroplane during landing to help the pilot. 
However this is only the part one of the project where the detection of the runway happens and the actual guidance needs to be provided based on this input. 

# The following tasks are done in this project: 
1. Capturing the image and providing it to a processing unit for runway detection.
2. A simple edge detection algorithm (Canny filter) is used here detect the edges. This helps in the detection of the runway. 
3. The image with the edges is then encrypted using a modified version of the basic RSA encryption algorithm. Along with the encryption, the image is compressed using a linear compression algorithm. This type of compression is possible since a simple bitmap image is used. However, this compression reduces the processing time tremendously. 
4. The encrypted and compressed image is now present in the aeroplane's controller unit. It is transmitted to the ground station where there is a server.
5. The server in the ground station recieves the input of the image with the edges and this ends the scope of our part one of this project. 
A simple TCP/IP protocol on top of FreeRTOS is implemented for the communication between aeroplane and the server.

It is to be noted that in real time, there will be a minimum number of frames to be processed and transmitted so that a proper guidance can be recieved. This will depend on the implementation of the part two of this project. Based on this requirement, an RTOS scheduler has to be designed. Appropriate hardware needs to be selected based on this processing as it's primary selection criteria.



# In part two: (to be implemented) 
This is not yet implemented and can be tried out. 
1. Using the decrypted images containing the edges of a captured image the actual runway can be detected. 
2. When this is done continuously for more than one image, the trajectory of the aeroplane can be computed. 
3. Based on the computed trajectory, a corrected trajectory for a smooth landing can be computed. (other parameters like external conditions, weather can be taken into account)
4. This computed trajectory needs to be converted into the control parameters of the aeroplane based on the model of the plane.
5. Finally, these control parameters can be sent back to the aeroplane in real time for it to auto adjust it's course and land smoothly. 

# How to use?
The details about the installation and usage can be found in the 'README- Usage Guide.md'
The configuration parameters, FreeRTOS tasks, their scheduling and sample outputs can be found in 'Autonomous Runway Detection Report.docx'
