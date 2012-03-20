pk is a framework that Joyent uses to maintain a defined binary package set
from a new pkgsrc jumpstart, to pruning stale packages and pushing out to
a HTTP server via rsync.

The framework comes with pkgsrc config files that help building on SmartOS,
some additional patches, and support files for the SMF functionality.

The 'pk' command is intended to deal with most of the daily tasks related
to package management, and the initial 'init' command starts off by hooking
up all the included config bits.

It needs to be said that pk is still work in progress, and some parts are
still largely Joyent Public Cloud specific (e.g. the hard wired URL of
'pkgsrc.joyent.com' as the public HTTP based repo). This will be fixed and
made configurable as time goes.

See 'pk help' for a basic description of the commands. Also note that some
config options can be pre-set in a ~/.pk file.

Patches are welcome, though the current aim is to only support SmartOS.

Some notes on the functionality included follow.


ZFS support
======================

One of the core advantages is gained by using ZFS file systems delegated
into the zone (machine) you're building inside of. Once a filesystem of
'base' (underneath the zone's root file system) is found, pk can use it
to generate a structure of prefix/release relative file systems, and allow
you to instantly reset to a clean table by rolling back to a 'start'
snapshot. This way you can reliably avoid stray files scattered around,
and make sure the file system environment is 100% consistent.

The 'base' filesystem should be available when you 'pk init' a new package
set (right now it involves manually delegating the file system to the zone,
from the host system (global zone).


The 'config' directory
========================

Just a bunch of config files organized by topic, that are included from
the pk.conf config file. This gets pulled by the 'pk init' command, which
injects the contents of fragment.mk into the default pkgsrc mk.conf file.


The 'lists' directory
========================

Holds a set of plain text files which contain packages to be built by pk.


The 'meta' directory
======================

This directory holds SMF support files, and INSTALL/ALTERNATIVES files.
For SMF functionality, you'll need to use the forked pkgsrc from:

  https://github.com/mamash/pkgsrc

By default, if a 'manifest.xml' file is found for a package, in a PKGPATH
directory (e.g. www/apache22) underneath SMFBASE (which defaults to 'meta'),
SMF support is triggered. Subsequently if 'method' file is found, it is used
as the SMF manifest's method script (unless the SMF method is already made
explicit in the manifest file). See examples. Some SMF variables can be
changed, see config/smf.inc.

Furthermore, if an 'INSTALL' file is found for the package, it is added to
the list of the package's install scripts (to be executed at pre/post install
time). This is so that install scripts can be added to packages without
tainting the pkgsrc tree.

If 'ALTERNATIVES' file is found for the package, it is used as the *single*
package's ALTERNATIVES file (pkgsrc doesn't support multiple ALTERNATIVES
sources yet). At Joyent, this is used to turn around the default GNU prefix
setup (GNU tools install prefix-less by default, but provide g-prefixed
alternatives via pkg_alternatives).


The 'patches' directory
==========================

This directory corresponds to the standard pkgsrc LOCALPATCHES functionality
and only contains few interesting entries so far (above all DTrace probes for
lang/erlang).

