#include <openssl/evp.h>

void auth_reply(char my_pubkey[], unsigned char my_secret[], char generator[], char their_pubkey[]);

int init_aes(unsigned char my_secret[], EVP_CIPHER_CTX* en, EVP_CIPHER_CTX* de);

void encrypt_msg(char msg[], EVP_CIPHER_CTX* en);

void decrypt_msg(char msg[], EVP_CIPHER_CTX* de);
