mediaRAID
=========

This was an attempt to make a JBOD-style software RAID system.  The idea was to hook up a bunch of USB disks (various brands/sizes) and turn them into a single pool of storage. 

The basic idea was to do the replication at the file level.  So let's say you have 5 disks and want triple-redundancy, you would make sure that a file was present on at least three of the disks.

There were two parts: the replication engine (that ensured files were mirrored properly).  I got this mostly working.  The second part was the multiplexing engine that made all of the disks act like a single blob.  This also kind of worked, but I could never get a good interface up in front of it.

I also could never get it reliable enough to run on my own system.  Maybe if I have a bunch of spare time I'll pick it up sometime.

Beware of the code.  It's written in C, and I'm sure there are bugs.


Example usage:

./mediaRAID -o default_permissions -o allow_other -log output.log -port 14444 tmp
wget -qO- -t 1 "http://localhost:14444/volume/add?basepath=/tmp/raid/t1"