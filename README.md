XXX turns out this was never meant to be a public API, though MapR did publish example code using it. XXX
XXX UNMAINTAINED XXX

To compile under MapR 5.2 the following must first be done, due to an error or ommision in the headers under /opt/mapr/include/streams in 5.2.

    $ sudo mkdir /opt/mapr/include/streams/common
    $ sudo cp /usr/include/errno.h /opt/mapr/include/streams/common/

Because of this, which is all that changed in these headers from 5.1 -> 5.2, neither common nor errno.h exist in 5.2

    $ diff -r include_streams_5.1/types.h /opt/mapr/include/streams/types.h
    11c11
    < #include <errno.h>
    ---
    > #include "common/errno.h"

XXX

strcat - poorly named utility to treat mapr streams as files

To even build this you need a MapR cluster, the MapR sandbox (https://www.mapr.com/products/mapr-sandbox-hadoop/download) will also work.  You can build and run strcat from a linux box with the MapR client installed, or from any cluster node.  You can not build unless you are either building on a cluster node or a computer with the mapr client installed.  I have only tested on linux, not OSX or Windows, though with a properly tweaked build.sh it should work on either.  You will need suitable development tools.  'yum -y groupinstall "Development Tools"' will work on centos (which the MapR sandbox VM uses), and "apt-get build-essential" should work on debian and derivatives, though I haven't tried to compile strcat anywhere but centos 6 and 7.

## Download the repository:

    git clone https://github.com/pbkdf3/strcat.git

## Build
    cd strcat
    ./build.sh

There may be some warnings.  note version number. (what #?, oh right there isn't one. HINT).

If stdin is a terminal, strcat will operate in consumer mode and read from the streamtopic regex provided as an argument.  If -x is used and in consumer mode, strcat will never exit, polling the stream and producing output until killed.  Otherwise it exits once the timeout is reached with no data, currently 1000ms.  You can change this in the consumer configuration in the code and recompile for now.  The timeout may be made an option in the future.  -x has no effect if in producer mode.  If -g is not specified an ephemeral consumer group id is assigned by the library (not by strcat).  -g has no effect in producer mode.

If stdin is not a terminal, strcat will operate in producer mode and write into the streamtopic specified.  Regexes are not valid in producer mode.  In producer mode, strcat reads lines from stdin via getline() and puts each in the topic specified.  newlines are not included in the messages put into the stream.

-p and -c allow forcing producer and consumer mode, useful in contexts where stdin/stdout may be captured by another process, even if not used, such as when running strcat under clustershell.  -c is good for these situations.  If trynig to insert messages into a topic from the console interactively, -p can be used.

-w [seconds] gives consumers using the same gid time to rebalance before starting, otherwise if starting multiple processes at once under the same group id, a single consumer may (will, in my experience) do all the work.  Userful when launching via clustershell over mulitple nodes, to for example grep a stream using your cluster.

## Use

    $ maprcli stream create -path /files
    Warning: produce/consume/topic permissions defaulting to creator. To change, execute 'maprcli stream edit -path /files -produceperm p -consumeperm p -topicperm p'

    # basic input/output
    $ find /usr -print | wc -l
    33832

    $ find /usr -print | strcat /files:usr

    $ strcat /files:usr | wc -l
    33832

    # can use the console (stdin as tty) to produce with -p
    $ strcat -p /files:hello
    hello, world!
    blah blah
    $ strcat /files:hello
    hello, world!
    blah blah

    # streaming/continous
    $ inotifywait -m -r /proc | strcat /files:events &
    [1] 5195
    Setting up watches.  Beware: since -r was given, this may take a while!
    Watches established.

    $ strcat -g notify /files:events | wc -l
    5692

    # wait a bit ..
    $ strcat -g notify /files:events | wc -l
    2307

    $ kill %1
    [1]+  Terminated              inotifywait -m -r /proc | strcat /files:events

    # make lots of messages (be mindful of your disks)
    $ find / -type f | strcat /files:all

    $ strcat /files:all | wc -l
    215899

    # hit ctrl-c after a bit (not too long!). pv is optional, this cluster is small vms...
    $ strcat -x /files:all | pv | strcat /files:all
    ^C99MiB 0:00:10 [  20MiB/s] [                     <=>

    $ strcat /files:all | wc -l
    5789787


	
    
    
    
    
    
