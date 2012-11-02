/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 13:53
#      Filename : crc16.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __CRC16_H
#define __CRC16_H

#include <stdint.h>
#include <sys/file.h>

short Caculate(const void *pdata,  int length);

#endif /* __CRC16_H */

