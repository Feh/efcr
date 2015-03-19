The Evil File Content Replacer
==============================

This small programs demonstrates it’s entirely feasible to exploit
a race condition like `some-program file > file.new && mv file.new file`.

Usage: `./efcr desired-input file-to-watch`

Example run:

    Terminal 1 (Attacker):                 Terminal 2 (Unknowing victim):
    ----------------------                 ------------------------------
    $ touch file
    $ echo "my evil content" > evil
    $ ./efcr evil file
    ...
     efcr: Setting up inotify watch...     $ echo "foo bar" >| file && cat file
     efcr: file was just closed after      my evil content
         writing! The race begins!
     efcr: Replacing contents...
     efcr: ... done.
     noticing close() after write and
         replacing with evil contents
         took 434703ns

So, just use `mktemp(1)` for this stuff.
