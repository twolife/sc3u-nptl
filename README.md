# SimCity™ 3000 Unlimited for modern Linux

This is a hack that allows you to run the old native Linux version of SC3U, released by [Loki Software](https://en.wikipedia.org/wiki/Loki_Entertainment) in 2000, on your current Linux distribution.

## Requirements

Make sure you have a 32-bit capable C compiler.

You are most likely running a Linux distribution on a shiny 64 bits CPU, but the binaries you are trying to run are build for the classic 32 bits PC architecture. In the Debian/Ubuntu world, you will need to install the *gcc-multilib* package.

You will also need to:
 - install the [OSS Proxy Daemon](https://github.com/libfuse/osspd)
 - install [patchelf](https://github.com/NixOS/patchelf)
 - download and install a [really old version of libstdc++](https://snapshot.debian.org/archive/debian/20060714T000000Z/pool/main/g/gcc-2.95/libstdc%2B%2B2.10-glibc2.2_2.95.4-27_i386.deb)
 - download a very old `libopenal-0.0.so` with a specific ABI, released in 2000. I can't even find source code for a such ancien version.
   All I could find is a binary version bundled at [https://www.improbability.net/loki/](https://www.improbability.net/loki/loki_compat_libs-1.5.tar.bz2). This version cames from the original UT99 release. Place it along side the game files.

## Usage

Build the code:

    $ make

Go to the game directory and make use of patchelf's blackmagic:

    $ patchelf --replace-needed libSDL-1.1.so.0 libSDL-1.2.so.0 sc3u.dynamic
    $ patchelf --replace-needed libSDL-1.1.so.0 libSDL-1.2.so.0 sc3bat.dynamic
    $ patchelf --add-needed libstdc++-libc6.2-2.so.3 ./sc3u.dynamic
    $ patchelf --add-needed libstdc++-libc6.2-2.so.3 ./sc3bat.dynamic

You can now run the game:

    $ LD_LIBRARY_PATH=$(pwd) LD_PRELOAD=/path/to/sc3u-nptl.so ./sc3u.dynamic

You can also run the Building Architect Plus tool: 

    $ LD_LIBRARY_PATH=$(pwd) LD_PRELOAD=/path/to/sc3u-nptl.so ./sc3bat.dynamic

## Disclaimer

This isn't heavily tested. But it worked for a few minutes on my computer and I was able to complete the "Tutorial on Getting Started" scenario.
