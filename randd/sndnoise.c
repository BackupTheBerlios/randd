/***************************************************************************
                          sndnoise.c  -  description
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

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#include "log.h"
#include "input.h"
#include "queue.h"

struct sound_t{
        input_handle hdl;
        struct queue_t *q;
        char*name,*fname;
        int bsize,sdiv,sch,bits;
};

#define S16L 0
#define S16B 1
#define S8   2

void soundcallback(input_handle hdl)
{
        struct sound_t *sp;
        sp=getinputhandledata(hdl);
        sp->q->mode=QREAD;
}


void soundhandler(int mode,struct queue_t *q)
{
        struct sound_t *sp;
        char*buf,*b;
        int bt,i;
        sp=q->data;
        if(!inputbufspace(sp->hdl)){
                q->mode=0;
                return;
        }
        buf=malloc(sp->bsize);
        if(read(q->fd,buf,sp->bsize)<sp->bsize){
                log(L_WARNING,"sound device delivered too small block on input %s",sp->name);
                free(buf);
                return;
        }
        b=malloc(sp->bsize>>3);
        for(i=0;i<(sp->bsize/sp->sdiv);i++){
                if((i&7)==0)b[i>>3]=0;
                bt=(buf[i*sp->sdiv+sp->sch]&1);
                b[i>>3]|=bt<<(i&7);
        }
        inputadd(sp->hdl,b,(sp->bsize/sp->sdiv)>>3);
        free(buf);
        free(b);
}









int create_sound(char*name,int bufsize,char*fname,int argc,char**argv)
{
        int fd,mode,c;
        struct sound_t*sp;
        struct queue_t *q;
        input_handle hdl;
        char*buf;

        fd=open(fname,O_RDONLY);
        if(fd<0){
                log(L_ERROR,"error: couldn't open sound device on input %s file %s",name,fname);
                return 0;
        }

        sp=malloc(sizeof(struct sound_t));

        /*reset device:*/
        if(ioctl(fd,SNDCTL_DSP_RESET,0)<0){
                log(L_ERROR,"unable to reset DSP of input %s file %s",name,fname);
                free(sp);
                close(fd);
                return 0;
        }
        if(ioctl(fd,SNDCTL_DSP_GETFMTS,&c)<0){
                log(L_ERROR,"unable to get sample size of input %s file %s",name,fname);
                free(sp);
                close(fd);
                return 0;
        }
        if(c&AFMT_S16_LE){mode=AFMT_S16_LE;sp->sdiv=2;sp->sch=0;log(L_DEBUG,"16bit Little Endian");}else
        if(c&AFMT_S16_BE){mode=AFMT_S16_BE;sp->sdiv=2;sp->sch=1;log(L_DEBUG,"16bit Little Endian");}else
        if(c&AFMT_U8){mode=AFMT_U8;sp->sdiv=1;sp->sch=0;log(L_DEBUG,"8bit");}else
                {
                        log(L_ERROR,"unknown sample format of input %s file %s",name,fname);
                        free(sp);
                        close(fd);
                        return 0;
                }
        if(ioctl(fd,SNDCTL_DSP_SETFMT,&mode)<0){
                log(L_ERROR,"cannot set sample size of input %s file %s",name,fname);
                free(sp);
                close(fd);
                return 0;
        }
        if((c=1,ioctl(fd,SNDCTL_DSP_STEREO,&c))>=0){
                log(L_DEBUG,"stereo");
                sp->sdiv*=2; /*use only left channel, since right normally mirrors left*/
        }else if((c=0,ioctl(fd,SNDCTL_DSP_STEREO,&c))>=0){
                log(L_DEBUG,"mono");
        }else{
                log(L_ERROR,"error: unable to set mono/stereo of input %s file %s",name,fname);
                free(sp);
                close(fd);
                return 0;
        }
        /*try to set a rate:*/
        if((c=48000,ioctl(fd,SNDCTL_DSP_SPEED,&c))<0)
        if((c=44100,ioctl(fd,SNDCTL_DSP_SPEED,&c))<0)
        if((c=32000,ioctl(fd,SNDCTL_DSP_SPEED,&c))<0)
        if((c=22050,ioctl(fd,SNDCTL_DSP_SPEED,&c))<0){
                log(L_ERROR,"error: unable to set speed of input %s file %s",name,fname);
                free(sp);
                close(fd);
                return 0;
        }
        log(L_DEBUG,"Speed=%i samples/s",c);
        sp->bsize=0;
        ioctl(fd,SNDCTL_DSP_GETBLKSIZE,&sp->bsize);
        if(sp->bsize<4||sp->bsize>65536){
                log(L_ERROR,"error: illogical blocksize %i of input %s file %s",sp->bsize,name,fname);
                free(sp);
                close(fd);
                return 0;
        }else{
                log(L_DEBUG,"blocksize=%i",sp->bsize);
        }
        if(ioctl(fd,SNDCTL_DSP_SYNC,0)<0){
                log(L_WARNING,"warning: unable to sync input %s file %s",name,fname);
        }

        log(L_INFO,"using 1 bit out of %i bytes; random rate = %i bit/s = %i byte/s on input %s file %s",
            sp->sdiv,c/sp->sdiv,c/sp->sdiv/8,name,fname);

        /*throw away some blocks, since these tend to be zero*/
        buf=malloc(sp->bsize);
        for(c=0;c<16;c++)
                read(fd,buf,sp->bsize);
        free(buf);

        /*create entries*/
        q=malloc(sizeof(struct queue_t));
        q->fd=fd;
        q->mode=QREAD;
        q->data=sp;
        q->handler=soundhandler;

        hdl=createinput(name,bufsize,soundcallback,sp);
        if(!hdl){
                log(L_ERROR,"unable to create sound input");
                free(sp);
                free(q);
                close(fd);
                return 0;
        }
        sp->q=q;
        sp->hdl=hdl;
        queue(q);

        return 1;
}

