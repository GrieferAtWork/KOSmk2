
### <b>KOS (Hooby) Operating System & Kernel</b> ###

Like others of its kind, KOS (<i>pronounced chaos</i>) is a home-made operating system build around the idea of sandboxing.

This is the second rendition of KOS, the first of which remains available at "https://github.com/GrieferAtWork/kos-old-".
Unlike the last one though, this time much more focus has been put on keeping the kernel core clean, actually implementing a modular system capable of loading kernel-level drivers/modules/libraries at runtime.

In addition, much more posix features have been implemented, such as a proper <b>/dev</b> filesystem, <b>unix-signals</b> and <b>PID-namespaces</b> alongside zombie processes wait()-ing to be killed (If you get my drift), as well as directing a large portion of the focus towards memory management that allows what I call ALOA (ALocateOnAccess) as well as LOA (LoadOnAccess) and COW (CopyOnWrite) alongside SWAP capabilities, allowing lazy mapping of files to memory, not only in user-space, but also within the kernel itself.

Another strong aspect are the much more posix-conformant system headers including <b>EVERYTHING</b> that glibc has to offer in those same headers. (even when many things are but stubs until I want to run something that actually needs them...)

Many of the core ideas of the old KOS remain, such as signal-based synchronization (which was been greatly improved and simplified), and the inclusion of the linker to be apart of the kernel (Thus allowing for much faster load times when running applications already in cache, as well as no redundancy when loader drivers vs. user-space applications).

But don't take my word for it. - You can actually <b>run BUSYBOX</b> and <b>NCURSES</b> on the thing!

And if you think that's pretty awesome, wait until you realize that this kernel can run and link <i>.exe</i> and <i>.dll</i> files <b>compiled for Windows</b>!

Like everything, this is another <b>one-person project</b>, with development (of this rendition) having started on <b>5.7.2017</b>.

Chaos|KOS - Even more chaotic that last time.

## Working ##
 - i486+ (Yes. Your machine can't be older than time itself...)
   - Protected-mode ring #3
   - Can run on <b>real hardware</b>
 - QEMU
 - multiboot/multiboot2
   - Kernel commandline support
 - dynamic memory
   - ALOA (ALocateOnAccess)
   - LOA (LoadOnAccess)
   - COW (CopyOnWrite)
   - SWAP (SWAPping memory to disk)
   - memory-mapped files for reading/writing
 - syscall
   - Highly linux-compatible using 0x80 and same ids/registers
 - Unix-compliant user-space interfaces
   - ANSI-compliant Terminal
   - <code>fork()</code>/<code>exec()</code>/<code>wait()</code>/<code>pipe()</code>
   - <code>mmap()</code>/<code>munmap()</code>/<code>mremap()</code>/<code>brk()</code>/<code>sbrk()</code>
   - <code>signal()</code>/<code>raise()</code>/<code>kill()</code>/<code>sigprocmask()</code>
     - terminate/suspend/resume support for <code>SIGKILL</code>, <code>SIGSTOP</code>, <code>SIGCONT</code>
   - <code>open()</code>/<code>read()</code>/<code>write()</code>/<code>lseek()</code>
   - <code>mount()</code>/<code>umount()</code>
   - <code>argc</code>/<code>argv</code>/<code>environ</code>
   - <code>dlopen()</code>/<code>dlclose()</code>/<code>dlsym()</code>
   - Can run <b>BUSYBOX</b> and <b>NCURSES</b>!
 - PS/2 keyboard input
   - Using a proper driver this time
   - Supports all scansets (#1, #2 and #3)
 - multitasking/scheduler
   - PID-based addressing, including a mountable <code>/proc</code> filesystem
   - Per-thread _everything_ (Page-directory, files, signals, you-name-it; just like linux...)
   - The only thing that what you call a process must have in common is a secondary, shared signal pool that is per-thread-group
   - SMP support
   - low-cpu/true idle (Using per-cpu IDLE threads)
 - ELF binaries/libraries (<i>no extension</i> / <b>.so</b>)
 - PE binaries/libraries (<b>.exe</b> / <b>.dll</b>)
   - <b>TRUE</b> cross-platform native execution of executables.
   - Natively run executables originally compiled for windows. (s.a.: "apps/hybrid_demo/main.exe")
 - Disk I/O
   - Builtin driver for BIOS-disk support allows for access to boot USB stick
   - Partition tables
     - MBR
     - EFI
   - Filesystem
     - FAT-12/16/32
       - Including write support & symlink extension (uses cygwin symlinks)
     - <code>/dev</code>
     - <code>/proc</code>
 - Modular kernel design (New features are loaded by drivers)

## Planned (As seen in the old KOS) ##
 - PE binaries/libraries (.exe / .dll)
 - cmos rtc driver (Currently 'date' uses PIT interrupts and is reset during boot)
 - local exception handling
 - <code>futex()</code>

## Planned (As not seen in the old KOS) ##
 - <code>/sys</code>
 - signal exception handling
 - IP-stack
 - WLAN support for "Atheros AR2427 Wireless" (That one's inside my test machine)


### Build Requirements ###
 - See <code>/INSTALL</code>



