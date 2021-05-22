# cpmbox

v0.0

* Work in progress (and there's a long way to go) *

cpmbox is an attempt to provide a set of unified file management
utilities for CP/M 2.2, that are more modern in usage 
than the original CP/M implementations. This work is inspired by
'BusyBox' for embedded Linux systems, although the design strategy
use by BusyBox is neither useful, nor appropriate, for CP/M.
The utilities are, or will be, vaguely Unix-y in their names and
command lines, although of course some deviations are unavoidable
with CP/M. Most obviously, CP/M does not support directories, but
does supported drives identified by a letter.

The repository consists of a number of C modules that implement
the various utilities (e..g, `ls.c`), and a number of supporting
libraries that they share.

At present, the only utilities implemented are `cat`, `cal`, 
`cp`, `du`, `find`, `hexdump`, `ls`,
`mv`, and `untar`. I've implemented `untar` in particular to make
it easier to transfer batches of files using, e.g., XModem.

## The utilities

`cat [/p] {files...}`

Display one or more text files. Uses paging if `/p` is set.  Wildcards can be
used in the filenames, in which case all matching files will be listed.
`cat` accepts file redirection, so this is a crude way to copy a file:

    cat < sourcefile > targetfile

`cp [/v] [/d] {from...} {to}` 

Copy a file to a file, or multiple files to a drive, or a drive to a drive. For
example: "cp \*.c b:" The source files can contain wildcards; if they do, or
there are multiple sources, then the last argument must be a drive, e.g., `c:`. 

If `/v` (verbose) is given, the utility displays the source and destination 
names.  With '/d' (dots), it outputs a dot for each sector written. 
This can be useful for ensuring that the copy is actually working, with
large files. In CP/M, of course, a "large" file is anything over about 20kB.

`cal [/m] {month} {year}`

Display a calendar for the specified month. If the year is less than
100, 2000 is added. So 2021 can be written as "21". In a Unix
system, this utility would default to displaying the current month's
calendar if the user didn't set a date. Unfortunately, CP/M has no 
standardized way to get the date. 

`du [/v] [/p] [files...]`

Reports disk space used, in sectors and bytes, by the specified
files. The file specification can include drive letters or
wildcards, e.g., `du a:*.c`. If no files are specified, `du` reports
file usage on the current drive, in which case its output is similar to 
that of the CP/M utility `stat`. If `/v` (verbose) is specified, 
individual files sizes are shown. `/p` selects paging mode. 

`find [/p] {pattern}`

Searches all drives for files matching the pattern. `/p` enables
paging mode. This is highly simplified version of the Unix utility
of the same name; most features of the unix `find` are either
impossible to implement, or unhelpful, on CP/M.

`hexdump [/cmp] [/o offset] [file]`

Writes a hex dump of a file or memory. The display is paged if `/p` is
given. The display starts at the offset given by the `/o` argument. 
If the argument starts with `#`, it is treated as hexadecimal. 

If no file is given, the utility either reads from standard input or,
if `/m` is given, from memory.

`ls [/lpsu] {files or drives...}`

Lists the contents of drives. Files are sorted by name order unless `/s` (size
order) or `/u` (unsorted) is set. Display will be paged if 
`/p` is set. If filenames
are given on the command line (possibly with wildcards), output is limited
to those files. With '/l' (long), the size and attributes are displayed.

`ls` is much slower than the built-in `dir`, but that's partly because
the utility itself has to be loaded from disk. However, the reformatting of
the output to more contemporary layout also takes a lot of CPU cycles.

`mv {from} {to}`

Rename `from` to `to`. Both files must be on the same drive. Broadly the
same as the built in ERASE, except that the command-line syntax is more
modern.

`untar [/v] [/t] {file.tar} [drive:]`

Unpack a Unix tarfile to a drive. If no drive is specified, the files are
unpacked to the current drive.  At present, only uncompressed tar files are
supported. All tarfile entries other than regular files are ignored; in
particular, CP/M has no support for directories, so the structure is flattened.

`/v' (verbose) shows the names of the files being unpacked. '/t' (table) shows
the tarfiles contents and sizes, but does not unpack anything. The tar format
being what it is, it's not really any quicker to display the contents 
that at it is to unpack them.

## Command-line switches

All commands respond to `/h` (help). Most respond to `/p` (paging).
Switches must be given before any filename arguments.

## Paging

Utilities with a page mode operate in the same way as the Unix
`more` utility. Pressing space shows the next page, pressing
enter the next line, and ctrl+c stops the listing.

## Redirection
All utilities support the Unix-like file redirection operators: `>` (redirect
output) and `<` (redirect input). For example, to output a hex dump to a 
file

    hexdump ... > file.hex

Not all utilities produce sufficient output that this exercise is worthwhile,
even if it is supported.

## Building

I build CP/M using an emulation environment on a Linux machine, 
although it will build under CP/M using the Aztec C compiler. 
I haven't yet provided a way to automate this on CP/M, and figuring
out which files make up which utility is rather fiddly, because there
are so many components. 

## Legal, etc 

I haven't thought about this at all, as yet. Please contact me if you
are interested in any of this stuff. Needless to say, no warranty
is implied.



