/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-22 13:27
#      Filename : Debug.h
#   Description : not thing...
#
=============================================================================*/

#define 	__DEBUG_MSG__ 		1

#ifdef __DEBUG_MSG__
	#define DEBUG_MSG(fmt,args...) fprintf(stderr,fmt, ## args)
#else
	#define DEBUG_MSG(fmt,args...)
#endif
