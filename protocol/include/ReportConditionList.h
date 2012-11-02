/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:01
#      Filename : ReportConditionList.h
#   Description : not thing...
#
=============================================================================*/

#ifndef __REPORT_C_LIST_H__
#define __REPORT_C_LIST_H__

#include "myList.h"
#include <stdint.h>
#include "ReportCondition.h"

void ReportCondition_Add(struct list_head *new, struct list_head *head);
void ReportCondition_Del(struct list_head *entry);
int  ReportCondition_Search(uint32_t index, struct list_head *phead, InZone_Report_Struct **Condition_node);
void ReportCondition_Del_All(struct list_head *phead);
int  TDSA_ReportCondition_Search(uint32_t index, struct list_head *phead, SpeedAnomaly_Report **Condition_node);

#endif 
