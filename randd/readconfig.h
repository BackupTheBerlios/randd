/***************************************************************************
                          readconfig.h  -  description
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

#ifndef RANDD_READCONFIG_H
#define RANDD_READCONFIG_H

int asbool(char*);
int asbytes(char*);
int asint(char*);
int asoct(char*);

char*aslower(char*);

void clog(char*,...);


#endif
