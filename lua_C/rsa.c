#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <stdio.h>
// lua头文件
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int padding = RSA_PKCS1_PADDING;

void printLastError()
{
    char * err = malloc(130);;
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    printf("ERROR: %s\n", err);
    free(err);
}

RSA * createRSA(unsigned char * key,int public)
{
    RSA *rsa= NULL;
    BIO *keybio ;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio==NULL)
    {
        printf( "Failed to create key BIO\n");
        return 0;
    }
    if(public)
    {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    }
    if(rsa == NULL)
    {
        printf( "Failed to create RSA\n");
    }

    return rsa;
}

static int public_encrypt(lua_State* L)
{
    const char* data = lua_tolstring(L, 1, NULL); //取第一个参数
    int data_len = strlen(data);
    const char* key = lua_tolstring(L, 2, NULL);
    const char* encrypted = malloc(sizeof(char) * 1024);
    RSA * rsa = createRSA(key,1);
    int result = RSA_public_encrypt(data_len, data, encrypted, rsa, padding);
    if(result != -1)
    {
        lua_pushlstring(L, encrypted, strlen(encrypted)); //设置第一个返回值
    }
    else
    {
        printLastError("c");
        lua_pushnil(L);
    }

    free(encrypted);
    return 1; //返回返回值的个数
}

static int private_decrypt(lua_State* L)
{
    const char* enc_data = lua_tolstring(L, 1, NULL);
    int data_len = strlen(enc_data);
    const char* key = lua_tolstring(L, 2, NULL);
    const char* decrypted = malloc(sizeof(char) * 1024);
    RSA * rsa = createRSA(key,0);
    int  result = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, padding);
    if(result != -1)
    {
        lua_pushlstring(L, decrypted, strlen(decrypted));
    }
    else
    {
        printLastError("c");
        lua_pushnil(L);
    }

    free(decrypted);
    return 1;
}

static int private_encrypt(lua_State* L)
{
    const char* data = lua_tolstring(L, 1, NULL);
    int data_len = strlen(data);
    const char* key = lua_tolstring(L, 2, NULL);
    const char* encrypted = malloc(sizeof(char) * 1024);
    RSA * rsa = createRSA(key,0);
    int result = RSA_private_encrypt(data_len, data, encrypted, rsa, padding);
    if(result != -1)
    {
        lua_pushlstring(L, encrypted, strlen(encrypted));
    }
    else
    {
        printLastError("c");
        lua_pushnil(L);
    }

    free(encrypted);
    return 1;
}

static int public_decrypt(lua_State* L)
{
    const char* enc_data = lua_tolstring(L, 1, NULL);
    int data_len = strlen(enc_data);
    const char* key = lua_tolstring(L, 2, NULL);
    const char* decrypted = malloc(sizeof(char) * 1024);
    RSA * rsa = createRSA(key,1);
    int  result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, padding);
    if(result != -1)
    {
        lua_pushlstring(L, decrypted, strlen(decrypted));
    }
    else
    {
        printLastError("c");
        lua_pushnil(L);
    }

    free(decrypted);
    return 1;
}

int luaopen_rsa(lua_State *L) {
    luaL_Reg luaLoadFun[] = {
        {"public_encrypt", public_encrypt},
        {"private_decrypt", private_decrypt},
        {"private_encrypt", private_encrypt},
        {"public_decrypt", public_decrypt},
        {NULL,NULL}
    };
    luaL_register(L, "rsa", luaLoadFun);
    return 1;
}