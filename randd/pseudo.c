/***************************************************************************
                          pseudo.c  -  description
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

#include "input.h"

#include <stdlib.h>
#include <sys/time.h>

struct pseudo_t{
        int seedinterv,seedpos;
};

static void pseudoseed()
{
        unsigned int sd;
        int sz;
        sz=getentropy(&sd,sizeof(sd));
        if(sz<sizeof(sd)){
                struct timeval tv;
                gettimeofday(&tv,0);
                sd!=tv.tv_usec;
        }
        srand(sd);
}

static void pseudocallback(input_handle hdl)
{
        struct pseudo_t *ps;
        int i,len;
        char *buf;
        ps=getinputhandledata(hdl);
        len=inputbufspace(hdl);
        buf=malloc(len);
        for(i=0;i<len;i++){
                if(ps->seedinterv==ps->seedpos){
                        pseudoseed();
                        ps->seedpos=0;
                }
                buf[i]=rand()&0xff;
        }
        inputadd(hdl,buf,len);
}

int create_pseudo(const char*name,int bufsize,int seedint)
{
        input_handle hdl,seed;
        struct pseudo_t*ps;
        ps=malloc(sizeof(struct pseudo_t));
        ps->seedinterv=seedint;
        ps->seedpos=seedint;/*force seeding on first call*/
        hdl=createinput(name,bufsize,pseudocallback,ps);
        if(!hdl) free(ps);
        else pseudocallback(hdl);
        return hdl!=0;
}

