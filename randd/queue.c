/***************************************************************************
                          queue.c  -  description
                             -------------------
    begin                : Sat Aug 3 2002
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

#include "queue.h"
#include "log.h"

#include <unistd.h>
#include <sys/types.h>

/*pool.c:*/
void poolupdate(void);

static struct queue_t *loopbase=0;

void queue(struct queue_t*q)
{
        if(loopbase==0){
                loopbase=q;
                q->next=q->prev=q;
        }else{
                q->next=loopbase;
                q->prev=loopbase->prev;
                loopbase->prev->next=q;
                loopbase->prev=q;
        }
        log(L_DEBUG,"queued %i",q);
}

void unqueue(struct queue_t*q)
{
        log(L_DEBUG,"dequeued %i",q);
        q->prev->next=q->next;
        q->next->prev=q->prev;
        if(q==loopbase){
                loopbase=loopbase->next;
                log(L_DEBUG,"shifted loopbase");
        }
        if(q==loopbase){
                loopbase=0;
                log(L_DEBUG,"deleted loopbase");
        }
        q->next=q->prev=q;
}

int mainloop()
{
        for(;;){
                struct queue_t*q;
                fd_set rd,wd,xd;
                int maxfd=-1,r;
                FD_ZERO(&rd);
                FD_ZERO(&wd);
                FD_ZERO(&xd);
                q=loopbase;

                if(!q){
                        log(L_ERROR,"no more streams to select on");
                        exit(1);
                }

                do{
                        log(L_DEBUG,"select check %i->%i",q->fd,q->mode);
                        if(q->fd>=0){
                                if(q->mode & QREAD){
                                        FD_SET(q->fd,&rd);
                                        maxfd=(maxfd<q->fd)?q->fd:maxfd;
                                }
                                if(q->mode & QWRITE){
                                        FD_SET(q->fd,&wd);
                                        maxfd=(maxfd<q->fd)?q->fd:maxfd;
                                }
                                if(q->mode & QEXCEPT){
                                        FD_SET(q->fd,&xd);
                                        maxfd=(maxfd<q->fd)?q->fd:maxfd;
                                }
                        }
                        q=q->next;
                }while(q!=loopbase);

                if(maxfd<0){
                        log(L_ERROR,"no file descriptors left to select on");
                        exit(1);
                }

                r=select(maxfd+1,&rd,&wd,&xd,0);

                q=loopbase;
                do{
                        int mode=0;
                        struct queue_t *q2=q->next;
                        if(q->fd>=0){
                                if(FD_ISSET(q->fd,&rd))mode=QREAD;
                                if(FD_ISSET(q->fd,&wd))mode|=QWRITE;
                                if(FD_ISSET(q->fd,&xd))mode|=QEXCEPT;

                                /*FIXME: might trigger too late*/
                                if((q->mode&QENTROPY)&&(getpoolsize()>0))mode|=QENTROPY;
                        }
                        if(mode)q->handler(mode,q);
                        q=q2;
                }while(q!=loopbase&&loopbase);

                poolupdate();
        }
}