strcat - poorly named utility to treat mapr streams as files

To even build this you need a MapR cluster, the MapR sandbox (https://www.mapr.com/products/mapr-sandbox-hadoop/download) will also work.  You can build and run strcat from a linux box with the MapR client installed, or from any cluster node.  You will need suitable development tools.  I work on Centos 7, 'yum -y groupinstall "Development Tools"' will work there (also on Centos 6, which the MapR 5.1 Sandbox uses).

## Download the repository:

    git clone https://xxx/xxx.git

## Build
    cd strcat*
    ./build.sh

There may be some warnings.  note version number.

If stdin is a terminal, strcat will consume from the streamtopic regex provided as an argument.  If -x is used, strcat will never exit.  Otherwise it exits once the timeout is reached with no data, currently 1000ms.  The timeout may be made an option in the future.  if -g is not specified an ephemeral consumer group id is assigned.

## Use

    [root@t3dn01 ~]# ./strcat
    usage:  ./strcat [-x] [-g gid] /stream:topic
    -x: do not exit on timeout, stream forever
    -g gid: consumer group id

    [root@t3dn01 ~]# maprcli stream create -path /ingest -defaultpartitions 4
    Warning: produce/consume/topic permissions defaulting to creator. To change, execute 'maprcli stream edit -path /ingest -produceperm p -consumeperm p -topicperm p'

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
    
    
    
	
    
    
    
    
    