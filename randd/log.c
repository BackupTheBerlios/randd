/***************************************************************************
                          log.c  -  description
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

#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>


static int loglevel=L_DEFAULT;
static int logfd=2;
static int logsys=0;


static int translev(int l)
{
        switch(l){
                case L_INFO:case L_INTERFACE:
                        return LOG_INFO;
                case L_DEBUG:case L_IODEBUG:
                        return LOG_DEBUG;
                case L_ERROR:
                        return LOG_ERR;
                case L_WARNING:
                        return LOG_WARNING;
                default:
                        return LOG_NOTICE;
        }
}

void log(int level,const char*format,...)
{
        char buf[2048];
        va_list va;

        if(!(level&loglevel))return;

        va_start(va,format);
        vsnprintf(buf,2046,format,va);
        va_end(va);

        if(logsys)syslog(translev(level),buf);
        else{
                strcat(buf,"\n");
                write(logfd,buf,strlen(buf));
        }
}

void setloglevel(int l)
{
        loglevel=l;
}

void setlogfd(int fd)
{
        if(logsys)closelog();
        logsys=0;
        logfd=fd;
}

void setlogsyslog()
{
        if(logsys)return;
        logsys=1;
        openlog("randd",LOG_PID,LOG_DAEMON);
}
