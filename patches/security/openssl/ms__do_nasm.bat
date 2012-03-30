$NetBSD$

--- ms/do_nasm.bat.orig	2008-09-18 11:56:09.000000000 +0000
+++ ms/do_nasm.bat
@@ -14,6 +14,7 @@ cd ..\..\..
 echo AES
 cd crypto\aes\asm
 perl aes-586.pl win32n %ASMOPTS% > a_win32.asm
+perl aesni-x86.pl win32n %ASMOPTS% > an_win32.asm
 cd ..\..\..
 
 echo DES
