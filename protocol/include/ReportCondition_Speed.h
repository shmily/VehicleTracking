/*=============================================================================
#
#        Author : shmily - shmily@mail.dlut.edu.cn
#            QQ : 723103903
# Last modified : 2012-10-18 10:01
#      Filename : ReportCondition_Speed.h
#   Description : not thing...
#
=============================================================================*/

#ifndef		_REPORT_CONDITION_SPEED_H_
#define		_REPORT_CONDITION_SPEED_H_

void TDSA_Condition_Initial(struct list_head *phead);

APP_Error TimeDepend_SpeedAnomaly_Report(CommandType *command);
APP_Error TDSA_Report_ACK(CommandType *command);
void ShowSpeedCondition(struct list_head *phead);
#endif
