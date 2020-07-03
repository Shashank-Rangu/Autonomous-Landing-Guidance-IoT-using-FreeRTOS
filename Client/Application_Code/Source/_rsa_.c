#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include "FreeRTOS.h"
#include "_canny_.h"
#include "_rsa_.h"

int* ce_cd();// computing the possible value of e & d pair
compressed_image rsa_encryption(pixel_t * msg, int msg_length)
{
    if (encryptionPrime1 == 0 || encryptionPrime2 ==0 ||prime(encryptionPrime1)==0 || prime(encryptionPrime2) == 0)exit(1);
    
    int * e_d=ce_cd();  
    compressed_image output = encrypt(msg, msg_length, e_d[0]);
    free(e_d);
    return output;
}
pixel_t* rsa_decryption(compressed_image msg)
{
    if (encryptionPrime1 == 0 || encryptionPrime2 == 0 || prime(encryptionPrime1) == 0 || prime(encryptionPrime2) == 0)exit(1);

    int* e_d = ce_cd();
    pixel_t* demsg= decrypt(msg, e_d[1]);
    free(e_d);
    return demsg;
}
int prime(long int pr)
{
    int j=sqrt((double)pr);// j is an integer square root value
    for(int i=2;i<=j;i++)
    {
        if(pr%i==0)
            return 0;
    }
    return 1;
}
int* ce_cd()
{
    long int t = (encryptionPrime1 - 1) * (encryptionPrime2 - 1);
    int *output = (int*)malloc(2*sizeof(int));
    for(int i=2;i<t;i++)
    {
        if(t%i==0)
            continue;
        if(prime(i)==1&&i!=encryptionPrime1&&i!=encryptionPrime2)
        {
            output[0]=i;
            long int k = 1;
            long int flag = 0;
            while (1)
            {
                k = k + t;
                if (k % output[0] == 0) {
                    flag = (k / output[0]);
                    break;
                }
            }
            if(flag>0)
            {
                output[1]=flag;
                break;
            }
        }
    }
    return output;
}
compressed_image encrypt(pixel_t* msg, int msg_length, long int key)
{
    int c = 0, flag = 0;
    if (msg[0] == 255) flag = 1;
    for (long int i = 0; i < msg_length-1; i++)
    {
        int c_count = 1;
        while (msg[i] == msg[i + 1])
        {
            c_count++;
            i++;
        }
        msg[c] = c_count;
        c++;
    }
    pixel_t* en_msg = (pixel_t*)pvPortMalloc((c+2) * sizeof(pixel_t));
    en_msg[c] = flag;// setting the starting number of the compressed data in the end
    
    long int n= encryptionPrime1 * encryptionPrime2;
    long int k;// uses the first possible value of e
    for (long int i = 0; i < c+1; i++)
    {
    k = 1;
    en_msg[i] = msg[i]+(i/512)+(i%512);//adding 2 so that values 0 and 1 also are consealed
    for(long int j=0;j<key;j++)// message to the power of the key
    {
        k=k*(en_msg[i]);
        k=k%n;
    }
    en_msg[i]=k;
    }
    free(msg);
    en_msg[c + 1] = 5000; // Storing a large enough value in the end to signify the end of data
    compressed_image en_msg_comp;
    en_msg_comp.en_msg = en_msg;
    en_msg_comp.en_msg_length = c+2;
    printf("last value of the compressed msg = %d \n", en_msg_comp.en_msg[en_msg_comp.en_msg_length-1]);
    return en_msg_comp;
}
pixel_t* decrypt(compressed_image comp_msg, long int key)
{
    
    int msg_length;
    pixel_t* msg = comp_msg.en_msg;
    msg_length = comp_msg.en_msg_length-1;
    long int n = encryptionPrime1 * encryptionPrime2;
    long int k; // using the first possible value of d
    for (long int i = 0; i < msg_length+1; i++)
    {
        
        k=1;
        for(long int j=0;j<key;j++)// cryptic text to the power of key 
        {
            k=k*msg[i];
            k=k%n;
        }
        msg[i] = k-(i/512)-(i%512);
        
    } //decryption is over

    //start the de-compression
    int actual_length = 0, flag = 0, counter = 0;
    for (int i = 0;i < msg_length;i++) {
        actual_length += msg[i];
    }// find out the actual message length
    if (comp_msg.en_msg[msg_length] == 1)flag = 255;
    pixel_t* de_msg = (pixel_t*)pvPortMalloc(actual_length * sizeof(pixel_t));
    for (int i = 0; i < msg_length; i++)
    {
        for (int j = 0; j < msg[i];j++) {
            de_msg[counter] = flag;
            counter++;
        }
        flag = flag == 0 ? 255 : 0;
    }
    return de_msg;
}