/***************************************************************************
                          egd.c  -  description
                             -------------------
    begin                : Sat Aug 10 2002
    copyright            : (C) 2002 by konrad
    email                : konrad@zaphod
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "egd.h"
#include "log.h"
#include "input.h"
#include "queue.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>


struct egdwrite_t{
        unsigned char pos,len,need,ignore;
        signed short cmd;
        char buffer[256];
};

static void egdout_free(struct queue_t*q)
{
        unqueue(q);
        shutdown(q->fd,SHUT_RDWR);
        close(q->fd);
        free(q->data);
        free(q);
}

static void egdout_handler(int mode,struct queue_t*q)
{
        struct egdwrite_t *e=q->data;
        if(mode&QEXCEPT){
                log(L_WARN,"Exception on EGD output stream, stopping it.");
                egdout_free(q);
                return;
        }
        if(mode&QENTROPY){
                if(e->need){
                        int s;
                        s=getentropy(e->buffer+e->pos+e->len,e->need);
                        if(s>0){
                                e->need-=s;
                                e->len+=s;
                                q->mode|=QWRITE;
                        }
                        if(!e->need)q->mode&= ~QENTROPY;
                }
        }

        if(mode&QREAD){
                if(e->pos>0){
                        log(L_WARN,"EGD-out stream: synchronousity violation: request while still answering, closing stream");
                        egdout_free(q);
                        return;
                }
                if(e->ignore){
                        int s;
                        s=read(q->fd,e->buffer,e->ignore);
                        if(s==0){
                                log(L_INFO,"EGD-out: end of file, closing.");
                                egdout_free(q);
                                return;
                        }
                        if(s<0){
                                log(L_WARN,"EGD-out: error on reading stream: %s",strerror(errno));
                                egdout_free(q);
                                return;
                        }
                        e->ignore-=s;
                        return;
                }
                switch(e->cmd){
                        case -1:{/*read command*/
                                int s;
                                unsigned char c;
                                s=read(q->fd,&c,1);
                                if(s==0){
                                        log(L_INFO,"EGD-out: End of file reading stream, closing.");
                                        egdout_free(q);
                                        return;
                                }
                                if(s<0){
                                        log(L_WARN,"EGD-out: error reading stream: %s",strerror(errno));
                                        egdout_free(q);
                                        return;
                                }
                                switch(c){
                                        case 0:{/*get entropy level*/
                                                int el;
                                                el=getpoolsize();
                                                e->cmd=-1;
                                                e->buffer[0]=(el>>24)&0xff;
                                                e->buffer[1]=(el>>16)&0xff;
                                                e->buffer[2]=(el>>8)&0xff;
                                                e->buffer[3]=el&0xff;
                                                e->len=4;
                                                q->mode|=QWRITE;
                                                break;
                                        }
                                        case 4:{/*get PID: denied for security*/
                                                e->buffer[0]=1;
                                                e->buffer[1]='0';
                                                e->len=2;
                                                q->mode|=QWRITE;
                                                break;
                                        }
                                        case 1:case 2:case 3:
                                                e->cmd=c;
                                                break;
                                        default:
                                                log(L_WARN,"EGD-out: error: unknown command sequence %i, closing.",(int)c);
                                                egdout_free(q);
                                                return;
                                }
                                break;
                        }
                        case 1:{/*read entropy nonblocking*/
                                unsigned char s;
                                int r;
                                e->cmd=-1;
                                r=read(q->fd,&s,1);
                                if(r==0){
                                        log(L_WARN,"EGD-out: encountered end-of-file during command sequence");
                                        egdout_free(q);
                                        return;
                                }
                                if(r<0){
                                        log(L_WARN,"EGD-out: error during reading command sequence: %s",strerror(errno));
                                        egdout_free(q);
                                        return;
                                }
                                e->len=getentropy(e->buffer+1,s);
                                e->buffer[0]=e->len;
                                if(e->len)q->mode|=QWRITE;
                                break;
                        }
                        case 2:{/*read entropy blocking: very bad idea...*/
                                unsigned char s;
                                int r;
                                e->cmd=-1;
                                r=read(q->fd,&s,1);
                                if(r==0){
                                        log(L_WARN,"EGD-out: encountered end-of-file during command sequence");
                                        egdout_free(q);
                                        return;
                                }
                                if(r<0){
                                        log(L_WARN,"EGD-out: error during reading command sequence: %s",strerror(errno));
                                        egdout_free(q);
                                        return;
                                }
                                e->len=getentropy(e->buffer,s);
                                if(e->len<s){
                                        e->need=s-e->len;
                                        q->mode|=QENTROPY;
                                }
                                if(e->len)q->mode|=QWRITE;
                                break;
                        }
                        case 3:case -30:case -31:{/*write entropy: ignored by this implementation*/
                                unsigned char s;
                                int r;
                                r=read(q->fd,&s,1);
                                if(r==0){
                                        log(L_WARN,"EGD-out: encountered end-of-file during command sequence");
                                        egdout_free(q);
                                        return;
                                }
                                if(r<0){
                                        log(L_WARN,"EGD-out: error during reading command sequence: %s",strerror(errno));
                                        egdout_free(q);
                                        return;
                                }
                                switch(e->cmd){
                                        case 3:e->cmd=-30;break;
                                        case -30:e->cmd=-31;break;
                                        case -31:e->cmd=-1;e->ignore=s;break;
                                }
                                break;
                        }
                        default: {/*internal error*/
                                log(L_ERROR,"EGD-out: internal error, command sequence was set to %i, closing.",(int)e->cmd);
                                egdout_free(q);
                                return;
                        }
                }
        }
        if(mode&QWRITE){
                int r;
                r=write(q->fd,e->buffer+e->pos,e->len);
                if(r<0){
                        log(L_WARN,"EGD-out: error writing stream: %s",strerror(errno));
                        egdout_free(q);
                        return;
                }
                e->pos+=r;
                e->len-=r;
                if(e->len<=0){
                        e->pos=0;
                        q->mode&= ~QWRITE;
                }
        }
}

static void egdout_listener(int mode,struct queue_t*q)
{
        int fd;
        struct queue_t *q2;
        fd=accept(q->fd,0,0);
        if(fd<0){
                log(L_WARNING,"lost connection before accepting it");
                return;
        }
        q2=malloc(sizeof(struct queue_t));
        q2->fd=fd;
        q2->data=malloc(sizeof(struct egdwrite_t));
        memset(q2->data,0,sizeof(struct egdwrite_t));
        ((struct egdwrite_t*)(q2->data))->cmd=-1;
        q2->mode=QREAD;
        q2->handler=egdout_handler;

        queue(q2);
}


int create_egdout_local(char*fname)
{
        int fd,r;
        struct sockaddr_un sa;
        struct queue_t *q;
        fd=socket(PF_LOCAL,SOCK_STREAM,0);
        if(fd<0){
                log(L_ERROR,"failed to get socket: %s",strerror(errno));
                return 0;
        }
        sa.sun_family=AF_UNIX;
        strncpy(sa.sun_path,fname,sizeof(sa.sun_path));
        r=bind(fd,&sa,sizeof(sa));
        if(r<0){
                log(L_ERROR,"failed to bind socket to %s: %s",fname,strerror(errno));
                close(fd);
                return 0;
        }
        if(listen(fd,5)<0){
                log(L_ERROR,"unable to listen on socket %s: %s",fname,strerror(errno));
                close(fd);
                return 0;
        }
        q=malloc(sizeof(struct queue_t));
        q->fd=fd;
        q->data=0;
        q->handler=egdout_listener;
        q->mode=QREAD;
        queue(q);

        return 1;
}

/*
int create_egdout_inet(char*hname,int port){}
int create_egdout_inet6(char*hname,int port){}


int create_egdin_local(char*fname){}
int create_egdin_inet(char*hname,int port){}
int create_egdin_inet6(char*hname,int port){}
*/