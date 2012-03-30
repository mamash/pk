$NetBSD$

--- crypto/engine/eng_all.c.orig	2010-03-01 00:30:11.000000000 +0000
+++ crypto/engine/eng_all.c
@@ -61,6 +61,8 @@
 
 void ENGINE_load_builtin_engines(void)
 	{
+	/* Engines may depend on CPU capabilities */
+	OPENSSL_cpuid_setup();
 	/* There's no longer any need for an "openssl" ENGINE unless, one day,
 	 * it is the *only* way for standard builtin implementations to be be
 	 * accessed (ie. it would be possible to statically link binaries with
@@ -71,6 +73,9 @@ void ENGINE_load_builtin_engines(void)
 #if !defined(OPENSSL_NO_HW) && !defined(OPENSSL_NO_HW_PADLOCK)
 	ENGINE_load_padlock();
 #endif
+#if !defined(OPENSSL_NO_HW) && !defined(OPENSSL_NO_HW_AESNI)
+	ENGINE_load_aesni();
+#endif
 	ENGINE_load_dynamic();
 #ifndef OPENSSL_NO_STATIC_ENGINE
 #ifndef OPENSSL_NO_HW
@@ -111,6 +116,7 @@ void ENGINE_load_builtin_engines(void)
 	ENGINE_load_capi();
 #endif
 #endif
+	ENGINE_register_all_complete();
 	}
 
 #if defined(__OpenBSD__) || defined(__FreeBSD__)
