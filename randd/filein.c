/***************************************************************************
                          filein.c  -  description
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

#include "filein.h"

#include "log.h"
#include "input.h"
#include "queue.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

struct filein_t{
        input_handle hdl;
        struct queue_t *q;
        char*name;
};

static void fileinhandler(int mode,struct queue_t*q)
{
        int max,len;
        char buf[2048];
        struct filein_t*fl;
        fl=q->data;
        max=inputbufspace(fl->hdl);
        if(max==0){
                q->mode=0;
                return;
        }
        if(max>sizeof(buf))max=sizeof(buf);
        len=read(q->fd,buf,max);
        if(len<0){
                log(L_WARNING,"got read error on input %s: %s",fl->name,strerror(errno));
                return;
        }
        if(len==0){
                log(L_WARNING,"got end of file on input %s",fl->name);
                return;
        }
        inputadd(fl->hdl,buf,len);
}

static void fileincallback(input_handle hdl)
{
        struct filein_t *fl;
        fl=getinputhandledata(hdl);
        fl->q->mode=QREAD;
}


int create_filein(char*name,int bufsize,char*fname)
{
        int fd;
        input_handle hdl;
        struct filein_t *fl;
        struct queue_t *q;

        fl=malloc(sizeof(struct filein_t));
        fd=open(fname,O_RDONLY);
        if(fd<0){
                log(L_WARNING,"error opening file %s: %s",fname,strerror(errno));
                free(fl);
                return 0;
        }
        q=malloc(sizeof(struct queue_t));

        q->fd=fd;
        q->mode=QREAD;
        q->handler=fileinhandler;
        q->data=fl;

        hdl=createinput(name,bufsize,fileincallback,fl);
        if(!hdl){
                log(L_WARNING,"cannot create input %s",name);
                free(fl);
                free(q);
                return 0;
        }
        queue(q);

        fl->q=q;
        fl->hdl=hdl;
        fl->name=malloc(strlen(name)+1);
        strcpy(fl->name,name);

        log(L_DEBUG,"created file input %s file %s on %i",name,fname,fd);

        return 1;
}

