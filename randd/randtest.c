/***************************************************************************
                          randtest.c  -  random tester
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

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int c1[2]={0,0},c2[4]={0,0,0,0},c4[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bit=0;

void scanbit(int b)
{
        bit<<=1;
        bit|=b&1;
        c1[bit&1]++;
        c2[bit&3]++;
        c4[bit&15]++;
}


main()
{
        int i,j;
        char c;
        for(i=0;i<10240;i++){
                if(read(0,&c,1)<0){
                        fprintf(stderr,"error: %s\n",strerror(errno));
                        exit(1);
                }
                for(j=0;j<8;j++)
                        scanbit(c>>j);
        }

        printf("0:1 %i:%i\n\n",c1[0],c1[1]);
        printf("00:01:10:11 %i:%i:%i:%i\n\n",c2[0],c2[1],c2[2],c2[3]);
        printf("0000:0001:0010:0011 %5i:%5i:%5i:%5i\n",c4[0],c4[1],c4[2],c4[3]);
        printf("0100:0101:0110:0111 %5i:%5i:%5i:%5i\n",c4[4],c4[5],c4[6],c4[7]);
        printf("1000:1001:1010:1011 %5i:%5i:%5i:%5i\n",c4[8],c4[9],c4[10],c4[11]);
        printf("1100:1101:1110:1111 %5i:%5i:%5i:%5i\n",c4[12],c4[13],c4[14],c4[15]);

}