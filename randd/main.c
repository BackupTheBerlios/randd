/***************************************************************************
                          main.c  -  description
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

/*config:*/
void scanconfig(const char*);
/*queue:*/
int mainloop();

#include <stdio.h>


int main(int argc,char**argv)
{
        if(argc>2){
                fprintf(stderr,"Usage: randd [configfile]\n");
                exit(1);
        }
        if(argc==2)scanconfig(argv[1]);
        else scanconfig("/etc/randd.cfg");

        return mainloop();
}