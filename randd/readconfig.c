/***************************************************************************
                          readconfig.c  -  description
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

#include "log.h"
#include "readconfig.h"
#include "input.h"
#include "filein.h"
#include "sndnoise.h"
#include "egd.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

/*inputs:*/
#include "pseudo.h"

/*outputs:*/
#include "pipeout.h"

#define MAXCONF 1024

static char **confv;
static int confc=-1;
static int cfgdaemon=0,cfgexit=1;

static char*content=0;
static int cpos=0;
static int cline=0;

void clog(char*str,...)
{
        char buf[2048];
        va_list va;
        va_start(va,str);
        vsnprintf(buf,2047,str,va);
        va_end(va);
        log(cfgexit?L_ERROR:L_WARNING,"config line %i: %s",cline,buf);
        if(cfgexit)exit(1);
}


static char*pack(char*s)
{
        char *t,*r;
        int sp=0;
        /*delete heading spaces*/
        while(*s==' '||*s=='\t')s++;
        if(!*s)return s;
        /*delete inner spaces*/
        for(r=t=s;*s;s++){
                if(sp){
                        if(*s==' '||*s=='\t');
                        else {*t++=*s;sp=0;}
                }else{
                        if(*s==' '||*s=='\t'){sp=1;*t++=' ';}
                        else{*t++=*s;}
                }
        }
        *t=0;
        /*delete trailing spaces*/
        if(*r){
                t=r+(strlen(r)-1);
                if(*t==' ')*t=0;
        }
        return r;
}


static void splitline(char*line)
{
        int i,l;
        confc=0;
        line=pack(line);


        if(!*line || *line=='#')return;

        l=strlen(line);
        confv[confc++]=line;

        for(i=0;i<l;i++){
                if(line[i]==' '){
                        confv[confc++]=line+i+1;
                        line[i]=0;
                        if(confc>=MAXCONF){
                                clog("too many items in config");
                        }
                }
        }
}

static void interpline()
{
        int poolsize=1024*1024;

        if(confc<=0)return;

        /*configuration*/
        if(!strcmp("logmethod",*confv)){
                if(confc<2||confc>3){clog("logmethod expects one or two parameters");return;}
                if(!strcmp("stderr",confv[1]))setlogfd(2);else
                if(!strcmp("stdout",confv[1]))setlogfd(1);else
                if(!strcmp("syslog",confv[1]))setlogsyslog();else
                if(!strcmp("file",confv[1])){
                        int fd;
                        if(confc<3){
                                clog("filename missing");
                        }else{
                                fd=open(confv[2],O_RDONLY);
                                if(fd<0){clog("cannot open file");}
                                else setlogfd(fd);
                        }
                }else clog("logmethod expects one of stderr,stdout,syslog,file");

        }else
        if(!strcmp("config",*confv)){
                if(confc!=2){clog("config expects either warning or error");return;}
                if(!strcmp("warning",confv[1]))cfgexit=0;else
                if(!strcmp("error",confv[1]))cfgexit=1;
                        else clog("config expects either warning or error");
        }else
        if(!strcmp("loglevel",*confv)){
                int i,lev=0;
                if(confc<2){clog("loglevel expects parameters");return;}
                for(i=1;i<confc;i++)
                        if(!strcmp("error",confv[i]))lev|=L_ERROR;else
                        if(!strcmp("warning",confv[i]))lev|=L_WARNING;else
                        if(!strcmp("info",confv[i]))lev|=L_INFO;else
                        if(!strcmp("debug",confv[i]))lev|=L_DEBUG;else
                        if(!strcmp("io",confv[i]))lev|=L_IO;else
                        if(!strcmp("interface",confv[i]))lev|=L_INTERFACE;else
                        if(!strcmp("iodebug",confv[i]))lev|=L_IODEBUG;else
                        if(!strcmp("config",confv[i]))lev|=L_CONFIG;else
                        if(!strcmp("default",confv[i]))lev|=L_DEFAULT;else
                        if(!strcmp("all",confv[i]))lev|=L_ALL;else
                        clog("unknown log level");
                setloglevel(lev);
        }else
        if(!strcmp("daemon",*confv)){
                if(confc!=2){clog("daemon expects boolean argument");}
                else cfgdaemon=asbool(confv[1]);
        }else

        /*input*/

        if(!strcmp("file",*confv)){
                if(confc!=4){clog("file expects input name, buffer size and file name");}
                else{
                        if(!create_filein(confv[1],asbytes(confv[2]),confv[3])){
                                clog("error creating file input");
                        }
                }
        }else
        if(!strcmp("sound",*confv)){
                if(confc<4){clog("sound requires at leas input name, bufsize, and device name");}
                else{
                        if(!create_sound(confv[1],asbytes(confv[2]),confv[3],confc-4,confv+4)){
                                clog("error creating sound input");
                        }
                }
        }else
        if(!strcmp("inegd",*confv)){
        }else
        if(!strcmp("inegdi",*confv)){
        }else
        if(!strcmp("inegdi6",*confv)){
        }else
        if(!strcmp("prand",*confv)){
                if(confc<3){clog("prand expects 2-3 arguments");}
                else {
                        int pint;
                        if(confc>3)pint=asbytes(confv[3]);
                        else pint=PSEUDO_DFLINTERV;
                        if(!create_pseudo(confv[1],asbytes(confv[2]),pint)){
                                clog("unable to create pseudo random input");
                        }
                }
        }else

        /*output*/

        if(!strcmp("pipe",*confv)){
                if(confc<2||confc>3){clog("pipe expects name and optional access mode");}
                else {
                        int acc=0600;
                        if(confc==3)acc=asoct(confv[2]);
                        if(!create_pipe(confv[1],acc)){
                                clog("unable to create pipe output");
                        }
                }
        }else
        if(!strcmp("socket",*confv)){
        }else
        if(!strcmp("egd",*confv)){
                if(confc!=2){clog("egd expects exactly one parameter");}
                else{
                        if(!create_egdout_local(confv[1])){
                                clog("unable to create local EGD output socket %s",confv[1]);
                        }
                }
        }else
        if(!strcmp("egdi",*confv)){
        }else
        if(!strcmp("egdi6",*confv)){
        }else

        /*pool*/

        if(!strcmp("persist",*confv)){
        }else
        if(!strcmp("poolsize",*confv)){
                if(confc!=2){clog("poolsize expects size argument");}
                else{
                        int ps=asbytes(confv[1]);
                        if(ps<=0){clog("poolsize must be positive");}
                        else poolsize=ps;
                }
        }else
        if(!strcmp("source",*confv)){
                if(confc<3){clog("source needs maxlevel and at least one input");}
                else{
                        if(!createsource(asint(confv[1]),confc-2,confv+2)){
                                clog("error creating source");
                        }
                }
        }else

        /*default: error*/
        {
                clog("unknown option");
        }

        initpool(poolsize);
}

static char*getline()
{
        char*ret=content+cpos;
        if(content[cpos]==0)return 0;
        for(;content[cpos]!='\n'&&content[cpos];cpos++);
        if(content[cpos])content[cpos++]=0;
        cline++;

        return ret;
}

/*used in main*/
void scanconfig(const char*file)
{
        int fd,clen;
        char*line;
        /*scan file*/
        confv=malloc(sizeof(char*)*MAXCONF);
        fd=open(file,O_RDONLY);
        if(fd<0){
                log(L_ERROR,"cannot open config file %s",file);
                exit(1);
        }
        clen=lseek(fd,0,SEEK_END);
        if(clen<0){
                log(L_ERROR,"error accessing file %s",file);
                exit(1);
        }
        lseek(fd,0,SEEK_SET);
        content=malloc(clen+1);
        read(fd,content,clen);
        content[clen]=0;
        close(fd);

        while((line=getline())){
                splitline(line);
                if(confc>0)interpline();
        }
        free(content);
        free(confv);

        /*checks*/
        if(cfgdaemon){
                log(L_INFO,"daemonizing");
                setlogsyslog();
                close(0);close(1);close(2);
                cfgdaemon=fork();
                if(cfgdaemon<0){
                        log(L_INFO,"error daemonizing, fork: %s",strerror(errno));
                        exit(1);
                }else if(cfgdaemon>0)exit(0);
                /*==0 is child and survives*/
        }

        /*ready*/
        log(L_INFO,"ready. Starting random gathering.");
}

char*aslower(char*s)
{
        char*r=s;
        for(;*s;s++)
                if(*s>='A'&&*s<='Z')*s+=('a'-'A');
        return r;
}


int asbool(char*s)
{
        aslower(s);
        if(!strcmp("yes",s)||!strcmp("y",s)||!strcmp("on",s)||
           !strcmp("true",s)||!strcmp("t",s)||!strcmp("1",s))
                return 1;
        else return 0;
}

int asint(char*s)
{
        return atoi(s);
}

int asoct(char*s)
{
        int ret=0;
        while(*s){
                if(*s<'0'||*s>'7')break;
                ret*=8;
                ret+=*s-'0';
        }
        return ret;
}

int asbytes(char*s)
{
        int l,i=atoi(aslower(s));
        l=strlen(s)-1;
        if(l<0)return 0;
        if(s[l]=='k')i*=1024;else
        if(s[l]=='m')i*=1024*1024;

        return i;
}
