/***************************************************************************
                          egdout.h  -  description
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

int create_egdout_local(char*fname);
int create_egdout_inet(char*hname,int port);
int create_egdout_inet6(char*hname,int port);


int create_egdin_local(char*fname);
int create_egdin_inet(char*hname,int port);
int create_egdin_inet6(char*hname,int port);


