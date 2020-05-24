#ifndef encryptionPrime1
#define encryptionPrime1 37
#endif
#ifndef encryptionPrime2
#define encryptionPrime2 41
#endif // !encryptionPrime2

typedef struct
{
    pixel_t* en_msg;
    int en_msg_length;
} compressed_image;

compressed_image encrypt( pixel_t*, int, long int);
pixel_t* decrypt(compressed_image, int, long int);

compressed_image rsa_encryption( pixel_t* msg, int msg_length);
pixel_t* rsa_decryption(compressed_image msg, int msg_length);
