$NetBSD$

--- lib/dtrace/c_src/dtrace_user.d.orig	2012-04-25 07:31:26.396024405 +0000
+++ lib/dtrace/c_src/dtrace_user.d
@@ -0,0 +1,39 @@
+/*
+ * %CopyrightBegin%
+ *
+ * Copyright Scott Lystig Fritchie 2011.
+ * All Rights Reserved.
+ *
+ * The contents of this file are subject to the Erlang Public License,
+ * Version 1.1, (the "License"); you may not use this file except in
+ * compliance with the License. You should have received a copy of the
+ * Erlang Public License along with this software. If not, it can be
+ * retrieved online at http://www.erlang.org/.
+ *
+ * Software distributed under the License is distributed on an "AS IS"
+ * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
+ * the License for the specific language governing rights and limitations
+ * under the License.
+ *
+ * %CopyrightEnd%
+ */
+
+provider erlang {
+    /*
+     * Two probes for use by Erlang code ... moved from here to
+     * erts/emulator/beam/erlang_dtrace.d until a more portable solution
+     * is found; see commit log for details.
+     *
+     * In an ideal world, we'd borrow code from the library at
+     * https://github.com/chrisa/node-dtrace-provider.  However, that
+     * library isn't portable to all of the platforms that DTrace is
+     * available on today, and it doesn't provide a solution for
+     * SystemTap users.
+     */
+};
+
+#pragma D attributes Evolving/Evolving/Common provider erlang provider
+#pragma D attributes Private/Private/Common provider erlang module
+#pragma D attributes Private/Private/Common provider erlang function
+#pragma D attributes Evolving/Evolving/Common provider erlang name
+#pragma D attributes Evolving/Evolving/Common provider erlang args
