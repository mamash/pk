$NetBSD$

--- util/mk1mf.pl.orig	2009-09-20 12:46:42.000000000 +0000
+++ util/mk1mf.pl
@@ -544,6 +544,8 @@ FIPSLINK=\$(PERL) util${o}fipslink.pl
 
 AES_ASM_OBJ=$aes_asm_obj
 AES_ASM_SRC=$aes_asm_src
+AESNI_ASM_OBJ=$aesni_asm_obj
+AESNI_ASM_SRC=$aesni_asm_src
 BN_ASM_OBJ=$bn_asm_obj
 BN_ASM_SRC=$bn_asm_src
 BNCO_ASM_OBJ=$bnco_asm_obj
@@ -805,6 +807,12 @@ foreach (values %lib_nam)
 			$lib_obj =~ s/\s\S*\/aes_cbc\S*//;
 			$rules.=&do_asm_rule($aes_asm_obj,$aes_asm_src);
 			}
+		if ($aesni_asm_obj ne "")
+			{
+			$lib_obj .= "\$(AESNI_ASM_OBJ) ";
+			#$lib_obj =~ s/\$\(AES_ASM_OBJ\)/\$(AES_ASM_OBJ) \$(AESNI_ASM_OBJ)/;
+			$rules.=&do_asm_rule($aesni_asm_obj,$aesni_asm_src);
+			}
 		if ($sha1_asm_obj ne "")
 			{
 			$lib_obj =~ s/\s(\S*\/sha1dgst\S*)/ $1 \$(SHA1_ASM_OBJ)/;
@@ -1135,6 +1143,7 @@ sub do_defs
 		elsif ($_ =~ /MD5_ASM/)	{ $t="$_ "; }
 		elsif ($_ =~ /SHA1_ASM/){ $t="$_ "; }
 		elsif ($_ =~ /AES_ASM/){ $t="$_ "; }
+		elsif ($_ =~ /AESNI_ASM/){ $t="$_ "; }
 		elsif ($_ =~ /RMD160_ASM/){ $t="$_ "; }
 		elsif ($_ =~ /CPUID_ASM/){ $t="$_ "; }
 		else	{ $t="$location${o}$_$pf "; }
