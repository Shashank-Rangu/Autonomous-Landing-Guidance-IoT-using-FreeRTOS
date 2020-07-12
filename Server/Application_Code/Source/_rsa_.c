#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include "_rsa_.h"
#include "FreeRTOS.h"
#include "_canny_.h"
int* ce_cd();// computing the possible value of e & d pair
pixel_t * rsa_encryption(pixel_t * msg, int msg_length)
{
    if (encryptionPrime1 == 0 || encryptionPrime2 ==0 ||prime(encryptionPrime1)==0 || prime(encryptionPrime2) == 0)exit(1);
    
    int * e_d=ce_cd();  
    encrypt(msg, msg_length, e_d[0]);
    free(e_d);
    return msg;
}
pixel_t* rsa_decryption(pixel_t* msg, int msg_length)
{
    if (encryptionPrime1 == 0 || encryptionPrime2 == 0 || prime(encryptionPrime1) == 0 || prime(encryptionPrime2) == 0)exit(1);

    int* e_d = ce_cd();
    decrypt(msg, msg_length, e_d[1]);
    free(e_d);
    return msg;
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
 pixel_t* encrypt(pixel_t* msg, int msg_length, long int key)
{
    long int n= encryptionPrime1 * encryptionPrime2;
    long int k;// uses the first possible value of e
    for (long int i = 0; i < msg_length; i++)
    {
        k = 1;
        msg[i] = msg[i]+(i/512)+(i%512);//adding 2 so that values 0 and 1 also are consealed
        for(long int j=0;j<key;j++)// message to the power of the key
        {
            k=k*(msg[i]);
            k=k%n;
        }
        msg[i]=k;
    }
    return msg;
}
pixel_t* decrypt(pixel_t* msg, int msg_length, long int key)
{
    long int n = encryptionPrime1 * encryptionPrime2;
    long int k; // using the first possible value of d
    for (long int i = 0; i < msg_length; i++)
    {
        
        k=1;
        for(long int j=0;j<key;j++)// cryptic text to the power of key 
        {
            k=k*msg[i];
            k=k%n;
        }
        msg[i] = k-(i/512)-(i%512);
        
    } 
    //free(en);
    //free(&msg_length);
    return msg;
}