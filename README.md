P2P File Transfer
===============

Peer to peer file transfer program which can share files between any number of clients 
(specified in the header file transfunc.h) with a server for initialization and update
of the peer list. The file is sent in chunks of 256kb and the I/O and incoming data is
multiplexed using SELECT function.

Implemented using C language standard header libraries (inet.h,ioctl.h,errno.h, etc)
