$NetBSD$

--- erts/emulator/beam/erl_bif_info.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_bif_info.c
@@ -109,6 +109,9 @@ static char erts_system_version[] = ("Er
 #ifdef VALGRIND
 				     " [valgrind-compiled]"
 #endif
+#ifdef HAVE_DTRACE
+				     " [dtrace]"
+#endif
 				     "\n");
 
 #define ASIZE(a) (sizeof(a)/sizeof(a[0]))
