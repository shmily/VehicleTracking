/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:28
#      Filename : SequenceNumber.h
#   Description : not thing...
#
=============================================================================*/

#ifndef _SEQ_NUMBER_H_
#define	_SEQ_NUMBER_H_

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void 		SEQ_Init	 (void);
void 		SEQ_Increase (void);
uint32_t 	Get_SEQ		 (void);


#endif
