/***************************************************************************
                          queue.h  -  description
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

#ifndef RANDD_QUEUE_H
#define RANDD_QUEUE_H

#define QNONE   0
#define QREAD   1
#define QWRITE  2
#define QEXCEPT 4
#define QENTROPY 8

#define QRDWR   3
#define QALL    15

struct queue_t{
        int fd;
        int mode;
        void (*handler)(int,struct queue_t*);
        void *data;

        /*internally used:*/
        struct queue_t *next,*prev;
};

void queue(struct queue_t*);
void unqueue(struct queue_t*);

#endif /* RANDD_QUEUE_H*/