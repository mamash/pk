$NetBSD$

--- util/pl/VC-32.pl.orig	2010-05-27 13:16:28.000000000 +0000
+++ util/pl/VC-32.pl
@@ -207,6 +207,7 @@ if ($nasm) {
 }
 
 $aes_asm_obj='';
+$aesni_asm_obj='';
 $bn_asm_obj='';
 $bn_asm_src='';
 $des_enc_obj='';
@@ -220,6 +221,8 @@ if (!$no_asm)
 	{
 	$aes_asm_obj='crypto\aes\asm\a_win32.obj';
 	$aes_asm_src='crypto\aes\asm\a_win32.asm';
+	$aesni_asm_obj='crypto\aes\asm\an_win32.obj';
+	$aesni_asm_src='crypto\aes\asm\an_win32.asm';
 	$bn_asm_obj='crypto\bn\asm\bn_win32.obj crypto\bn\asm\mt_win32.obj';
 	$bn_asm_src='crypto\bn\asm\bn_win32.asm crypto\bn\asm\mt_win32.asm';
 	$bnco_asm_obj='crypto\bn\asm\co_win32.obj';
@@ -248,6 +251,8 @@ if (!$no_asm)
 	{
 	$aes_asm_obj='$(OBJ_D)\aes-x86_64.obj';
 	$aes_asm_src='crypto\aes\asm\aes-x86_64.asm';
+	$aesni_asm_obj='$(OBJ_D)\aesni-x86_64.obj';
+	$aesni_asm_src='crypto\aes\asm\aesni-x86_64.asm';
 	$bn_asm_obj='$(OBJ_D)\x86_64-mont.obj $(OBJ_D)\bn_asm.obj';
 	$bn_asm_src='crypto\bn\asm\x86_64-mont.asm';
 	$sha1_asm_obj='$(OBJ_D)\sha1-x86_64.obj $(OBJ_D)\sha256-x86_64.obj $(OBJ_D)\sha512-x86_64.obj';
