/***************************************************************************
                          input.c  -  description
                             -------------------
    begin                : Sun Aug 4 2002
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

#define RANDD_INPUT_C 1
#include "input.h"
#include "log.h"

#include <stdlib.h>

struct input_t {
        char*name;
        char*buffer;
        int bstart,blen,bsize;
        bufferlow callback;
        void*data;
        struct input_t *next,*prev;
};


static struct input_t *inputbase=0;

struct source_t {
        int level,cnt;
        struct input_t **input;

        struct source_t *next,*prev;
};

static struct source_t *sourcebase=0;

static input_handle getinput(const char*name)
{
        struct input_t *i;
        if(inputbase==0)return 0;
        i=inputbase;
        do{
                if(!strcmp(name,i->name))return i;
                i=i->next;
        }while(i!=inputbase);
        return 0;
}


static char*pool=0;
static int psize=0,pstart=0,plen=0;


input_handle createinput(char*name,int bsize,bufferlow callb,void*data)
{
        input_handle hdl;
        if(getinput(name))return 0;

        hdl=malloc(sizeof(struct input_t));
        if(inputbase){
                hdl->next=inputbase;
                hdl->prev=inputbase->prev;
                inputbase->prev=hdl;
                hdl->prev->next=hdl;
        }else{
                inputbase=hdl->next=hdl->prev=hdl;
        }
        hdl->bsize=bsize;
        hdl->buffer=malloc(bsize);
        hdl->bstart=hdl->blen=0;
        hdl->callback=callb;
        hdl->data=data;
        hdl->name=malloc(strlen(name)+1);
        strcpy(hdl->name,name);

        log(L_DEBUG,"created input %s, buffer size %i",name,bsize);

        return hdl;
}

void*getinputhandledata(input_handle hdl)
{
        return hdl->data;
}

int inputbufspace(input_handle hdl)
{
        return hdl->bsize-hdl->blen;
}


int inputadd(input_handle hdl,void*buf,int len)
{
        int p,bp;
        bp=hdl->bstart+hdl->blen;
        if(bp>=hdl->bsize)bp-=hdl->bsize;
        for(p=0;p<len;p++){
                if(hdl->bsize==hdl->blen)break;
                hdl->buffer[bp++]=((char*)buf)[p];
                hdl->blen++;
                if(bp>=hdl->bsize)bp-=hdl->bsize;
        }

        return p;
}

void deleteinput(input_handle hdl)
{
        if(hdl==inputbase)inputbase=hdl->next;
        if(hdl==inputbase)inputbase=0;
        hdl->next->prev=hdl->prev;
        hdl->prev->next=hdl->next;
        free(hdl->buffer);
        free(hdl);
}

int createsource(int lvl,int inpc,char**inpv)
{
        struct source_t *src;
        int i;
        int ok=1;
        /*create source*/
        src=malloc(sizeof(struct source_t));
        src->level=lvl;
        src->cnt=inpc;
        src->input=malloc(inpc*sizeof(void*));
        for(i=0;i<inpc;i++){
                ok&=(src->input[i]=getinput(inpv[i]))!=0;
        }
        if(!ok){
                free(src->input);
                free(src);
                log(L_WARNING,"could not create source, missing at least one input");
                return 0;
        }
        /*sort into list*/
        if(sourcebase==0){
                sourcebase=src->next=src->prev=src;
        }else{
                struct source_t*tmp=sourcebase;
                do{
                        if(src->level>tmp->level)break;
                        tmp=tmp->next;
                }while(tmp!=sourcebase);
                src->next=tmp;
                src->prev=tmp->prev;
                tmp->prev->next=src;
                tmp->prev=src;
                if(sourcebase->level<src->level)sourcebase=src;
        }
        return 1;
}

int getentropy(void*buf,int len)
{
        int xlen,i;
        xlen=len>plen?plen:len;
        for(i=0;i<xlen;i++){
                ((char*)buf)[i]=pool[pstart++];
                plen--;
                if(pstart>=psize)pstart-=psize;
        }
        return xlen;
}

static void fillpool(struct source_t *src,int max)
{
        char buf[max];
        int i,j;

        /*init buffer*/
        for(i=0;i<max;i++)buf[i]=0;

        /*fill xor'ed buffer*/
        for(i=0;i<src->cnt;i++){
                struct input_t *tmp=src->input[i];
                for(j=0;j<max;j++){
                        buf[j]^=tmp->buffer[tmp->bstart++];
                        tmp->blen--;
                        if(tmp->bstart>=tmp->bsize)tmp->bstart=0;
                }
                tmp->callback(tmp);
        }
        /*transfer buffer into pool*/
        i=pstart+plen;
        if(i>psize)i-=psize;
        for(j=0;j<max;j++){
                pool[i++]=buf[j];
                plen++;
                if(i>psize)i-=psize;
        }
}

void poolupdate(void)
{
        int fgrade,i;
        struct source_t*tmp;
        log(L_DEBUG,"pool update");
        if(psize==plen)return;
        tmp=sourcebase;
        if(tmp)
        do{
                fgrade=plen*100/psize;
                log(L_DEBUG,"srclevel=%i fill=%i",tmp->level,fgrade);
                if(tmp->level>fgrade){
                        /*get maximum number of bytes to fetch from there*/
                        int max=psize*tmp->level/100-plen;

                        /*get actual maximum fill of all inputs of this source*/
                        for(i=0;i<tmp->cnt;i++)
                                if(tmp->input[i]->blen<max)
                                        max=tmp->input[i]->blen;

                        /*fill pool*/
                        if(max>0)fillpool(tmp,max);
                }
                tmp=tmp->next;
        }while(tmp!=sourcebase);
}

void initpool(int size)
{
        psize=size;
        pool=malloc(size);
}

int getpoolsize()
{
        return plen;
}
