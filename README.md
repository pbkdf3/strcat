strcat - poorly named utility to treat mapr streams as files

To even build this you need a MapR cluster, the MapR sandbox (https://www.mapr.com/products/mapr-sandbox-hadoop/download) will also work.  You can build and run strcat from a linux box with the MapR client installed, or from any cluster node.  You can not build unless you are either building on a cluster node or a computer with the mapr client installed.  I have only tested on linux, not OSX or Windows, though with a properly tweaked build.sh it should work on either.  You will need suitable development tools.  'yum -y groupinstall "Development Tools"' will work on centos (which the MapR sandbox VM uses), and "apt-get build-essential" should work on debian and derivatives, though I haven't tried to compile strcat anywhere but centos 6 and 7.

## Download the repository:

    git clone https://xxx/xxx.git

## Build
    cd strcat*
    ./build.sh

There may be some warnings.  note version number. (what #?, oh right there isn't one. HINT).

If stdin is a terminal, strcat will operate in consumer mode and read from the streamtopic regex provided as an argument.  If -x is used and in consumer mode, strcat will never exit, polling the stream and producing output until killed.  Otherwise it exits once the timeout is reached with no data, currently 1000ms.  You can change this in the consumer configuration in the code and recompile for now.  The timeout may be made an option in the future.  -x has no effect if in producer mode.  If -g is not specified an ephemeral consumer group id is assigned.  -g has no effect in producer mode.

If stdin is not a terminal, strcat will operate in producer mode and write into the streamtopic specified.  Regexes are not valid in producer mode.  In producer mode, strcat reads lines from stdin via getline() and puts each in the topic specified.  newlines are not included in the messages put into the stream.

-p and -c allow forcing producer and consumer mode, useful in contexts where stdin/stdout may be captured by another process, even if not used, such as when running strcat under clustershell.  Or if trynig to insert messages into a topic from the console interactively.

## Use


    [root@t3dn01 ~]# maprcli stream create -path /ingest -defaultpartitions 4
    Warning: produce/consume/topic permissions defaulting to creator. To change, execute 'maprcli stream edit -path /ingest -produceperm p -consumeperm p -topicperm p'

    [root@t3dn01 ~]# ./strcat
    usage: ./strcat [-xpc] [-g gid] /streamtopic
       -x: do not exit on timeout, stream forever
       -p: produce, stdin -> /stream:topic
       -c: consume, /stream:regex -> stdout
       -g: gid: consumer group id

    [root@t3dn01 ~]# time find / -type f | ./strcat /ingest:20160725
    real	     0m1.818s
    user	     0m1.710s
    sys	     0m1.880s
	
    [root@t3dn01 ~]# time ./strcat /ingest:20160725 | wc -l
    343589
    real	0m2.267s
    user	0m0.894s
    sys	0m2.377s
	
    [root@t3dn01 ~]# ./strcat /ingest:20160725 | head
    /boot/grub2/i386-pc/ohci.mod
    /boot/grub2/i386-pc/part_acorn.mod
    /boot/grub2/i386-pc/part_amiga.mod
    /boot/grub2/i386-pc/part_apple.mod
    /boot/grub2/i386-pc/part_bsd.mod
    /boot/grub2/i386-pc/part_dfly.mod
    /boot/grub2/i386-pc/part_dvh.mod
    /boot/grub2/i386-pc/part_gpt.mod
	/boot/grub2/i386-pc/part_msdos.mod
    /boot/grub2/i386-pc/part_plan.mod
    
    [root@t3dn01 ~]# time ./strcat /ingest:20160725 | ./strcat /ingest:copy
    real	     0m2.437s
    user	     0m2.132s
	sys	     0m2.775s
    
    [root@t3dn01 ~]# ./strcat '/ingest:copy' | head
    /proc/sys/dev/mac_hid/mouse_button_emulatio
    /proc/sys/dev/parport/default/spintim
    /proc/sys/dev/parport/default/timeslic
    /proc/sys/dev/parport/parport0/autoprob
    /proc/sys/dev/parport/parport0/autoprobe
    /proc/sys/dev/parport/parport0/autoprobe
    /proc/sys/dev/parport/parport0/autoprobe
	/proc/sys/dev/parport/parport0/autoprobe
    /proc/sys/dev/parport/parport0/base-add
    /proc/sys/dev/parport/parport0/devices/activ
    
    
    
	
    
    
    
    
    