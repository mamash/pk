$NetBSD$

--- crypto/engine/engine.h.orig	2010-02-09 14:18:15.000000000 +0000
+++ crypto/engine/engine.h
@@ -337,6 +337,7 @@ void ENGINE_load_sureware(void);
 void ENGINE_load_ubsec(void);
 #endif
 void ENGINE_load_cryptodev(void);
+void ENGINE_load_aesni(void);
 void ENGINE_load_padlock(void);
 void ENGINE_load_builtin_engines(void);
 #ifdef OPENSSL_SYS_WIN32
