/***************************************************************************
                          pipeout.c  -  description
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

#include "pipeout.h"
#include "queue.h"
#include "input.h"
#include "log.h"

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>


static void pipehandler(int mode,struct queue_t*q)
{
        char buf[128];
        int sz;
        sz=getentropy(buf,128);
        if(sz>0)write(q->fd,buf,sz);
}

int create_pipe(const char*name,int acc)
{
        int fd;
        struct queue_t *q;
        if(mknod(name,S_IFIFO|(acc&07777),0)){
                int e=errno;
                if(e!=EEXIST){
                        log(L_WARNING,"error creating pipe %s: %s",name,strerror(e));
                        return 0;
                }
        }
        fd=open(name,O_RDWR);
        if(fd<0){
                log(L_WARNING,"error opening pipe %s: %s",name,strerror(errno));
                return 0;
        }
        q=malloc(sizeof(struct queue_t));
        q->mode=QWRITE;
        q->fd=fd;
        q->data=0;
        q->handler=pipehandler;
        queue(q);

        log(L_DEBUG,"created output pipe %s on %i",name,fd);

        return 1;
}
