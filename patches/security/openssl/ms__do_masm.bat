$NetBSD$

--- ms/do_masm.bat.orig	2009-03-09 12:17:56.000000000 +0000
+++ ms/do_masm.bat
@@ -11,6 +11,7 @@
 @echo AES
 @cd crypto\aes\asm
 @perl aes-586.pl win32 %ASMOPTS% > a_win32.asm
+@perl aesni-x86.pl win32 %ASMOPTS% > an_win32.asm
 @cd ..\..\..
 
 @echo DES
