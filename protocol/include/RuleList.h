/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:01
#      Filename : RuleList.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __RULE_LIST_H__
#define __RULE_LIST_H__

#include "myList.h"

void Rule_Add(struct list_head *new, struct list_head *head);
void Rule_Del(struct list_head *entry);
int  Rule_Search(uint8_t *id, struct list_head *phead, Rule_Struct **Rule_node);
void Rule_Del_All(struct list_head *phead);

#endif 
