$NetBSD$

--- crypto/evp/evp.h.orig	2008-09-17 17:11:00.000000000 +0000
+++ crypto/evp/evp.h
@@ -955,6 +955,7 @@ void ERR_load_EVP_strings(void);
 /* Error codes for the EVP functions. */
 
 /* Function codes. */
+#define EVP_F_AESNI_INIT_KEY				 163
 #define EVP_F_AES_INIT_KEY				 133
 #define EVP_F_ALG_MODULE_INIT				 138
 #define EVP_F_CAMELLIA_INIT_KEY				 159
