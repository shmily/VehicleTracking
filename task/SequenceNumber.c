/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 09:29
#      Filename : SequenceNumber.c
#   Description : not thing...
#
=============================================================================*/

#include "SequenceNumber.h"

volatile uint32_t	_SEQ_NUM;

void SEQ_Init(void)
{
	_SEQ_NUM = 0;
}

void SEQ_Increase(void)
{
	_SEQ_NUM = (uint32_t)(_SEQ_NUM + 1);
}

uint32_t Get_SEQ(void)
{
	SEQ_Increase();
	return _SEQ_NUM;
}
