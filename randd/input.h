/***************************************************************************
                          input.h  -  description
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

#ifndef RANDD_INPUT_H
#define RANDD_INPUT_H

#ifdef RANDD_INPUT_C
typedef struct input_t* input_handle;
#else
typedef void* input_handle;
#endif
typedef void (*bufferlow)(input_handle);

input_handle createinput(char*name,int bufsize,bufferlow callback,void*data);
void*getinputhandledata(input_handle);
/*input_handle getinput(const char*name);*/
int inputbufspace(input_handle);
int inputadd(input_handle,void*,int);
/*deprecated: it does currently not clean up the sources:*/
void deleteinput(input_handle);

/**returns 1 if ok, 0 if error*/
int createsource(int lvl,int inpc,char**inpv);


int getentropy(void*,int);

int getpoolsize();


/*config*/
void initpool(int size);

#endif
