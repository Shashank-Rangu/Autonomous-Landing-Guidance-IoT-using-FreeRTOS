#ifndef encryptionPrime1
#define encryptionPrime1 37
#endif
#ifndef encryptionPrime2
#define encryptionPrime2 41
#endif // !encryptionPrime2

typedef short int pixel_t;


pixel_t * encrypt( pixel_t*, int, long int);
pixel_t* decrypt(pixel_t*, int, long int);

 pixel_t* rsa_encryption( pixel_t* msg, int msg_length);
pixel_t* rsa_decryption(pixel_t* msg, int msg_length);
