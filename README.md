# Mocker
A docker clone

This is based on the [How to create your own containers](https://cesarvr.io/post/2018-05-22-create-containers/) blog post.
Full disclosure, I attempted each section by myself before refering to his source code.

This was done purely as a learning experience and is my work supplimented largly by Cesar Valdez's work.

## Run
`make -s && sudo ./container`

The first time ran, you need to download and extract the alpine os.
I did not think making the user pull the entire OS along with the container source was a good idea, although I would love feedback on this (open an issue if you have feedback!)
```
mkdir root && cd root
curl -Ol http://nl.alpinelinux.org/alpine/v3.7/releases/x86_64/alpine-minirootfs-3.7.0-x86_64.tar.gz
tar -xvf alpine-minirootfs-3.7.0_rc1-x86_64.tar.gz
rm alpine-minirootfs-3.7.0_rc1-x86_64.tar.gz
```

## Implentation details

### Chroot and Alpine
The container will use chroot to isolate the filesystem, which means we will lose linux commands inside the container. To combat this, we will run an alpine instance inside the container as our new root.

### Limited Process Creation
I thought it would be an interesting exercize to limit the number of proccesses that could be created by the container.
Currently it is limitted to 5 process at a time, although 2 of them are taken up by the actual container, and bin/sh. So the user gets 3 processes to play with. This can be changed by changing the NUM_PROCESSES_ALLOWED def.
