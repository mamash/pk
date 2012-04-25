$NetBSD$

--- erts/emulator/beam/erl_nif.h.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/erl_nif.h
@@ -221,6 +221,10 @@ ERL_NIF_INIT_DECL(NAME)			\
 }                                       \
 ERL_NIF_INIT_EPILOGUE
 
+#ifdef HAVE_DTRACE
+ERL_NIF_TERM erl_nif_user_trace_s1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
+ERL_NIF_TERM erl_nif_user_trace_i4s4(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
+#endif
 
 #endif /* __ERL_NIF_H__ */
 
