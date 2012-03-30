$NetBSD$

--- ms/do_fips.bat.orig	2008-09-18 12:13:54.000000000 +0000
+++ ms/do_fips.bat
@@ -42,6 +42,8 @@ echo AES
 cd crypto\aes\asm
 perl aes-586.pl win32n %ASMOPTS% > a_win32.asm
 if ERRORLEVEL 1 goto error
+perl aesni-x86.pl win32n %ASMOPTS% > an_win32.asm
+if ERRORLEVEL 1 goto error
 cd ..\..\..
 
 echo DES
@@ -144,6 +146,8 @@ echo AES
 cd crypto\aes\asm
 perl aes-x86_64.pl aes-x86_64.asm
 if ERRORLEVEL 1 goto error
+perl aesni-x86_64.pl aesni-x86_64.asm
+if ERRORLEVEL 1 goto error
 cd ..\..\..
 
 echo SHA
