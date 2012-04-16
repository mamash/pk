$NetBSD$

--- erts/emulator/beam/erl_driver.h.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_driver.h
@@ -614,6 +614,8 @@ EXTERN int erl_drv_getenv(char *key, cha
 
 #endif
 
+/* also in global.h, but driver's can't include global.h */
+void dtrace_drvport_str(ErlDrvPort port, char *port_buf);
 
 
 
