/***************************************************************************
                          log.h  -  description
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

#ifndef RANDD_LOG_H
#define RANDD_LOG_H

#define L_NONE          0
#define L_ERROR         1
#define L_WARNING       2
#define L_CONFIG        4
#define L_INTERFACE     8
#define L_IO            16
#define L_DEBUG         32
#define L_IODEBUG       64
#define L_INFO          128

#define L_DEFAULT       3
#define L_ALL           65535

/*aliases:*/
#define L_WARN  L_WARNING
#define L_ERR   L_ERROR

void log(int level,const char*format,...);

void setloglevel(int);
void setlogfd(int);
void setlogsyslog();

#endif