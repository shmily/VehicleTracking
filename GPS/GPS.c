#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/time.h>
#include <string.h>

#include "nmea/GPS.h"
#include "uart.h"
#include "nmea/nmea.h"

nmeaGPRMC		vGPRMC;
GSM_Device_Uart GpsDevice;
char			Gps_str[512];
int				len;


void GPS_Init(void)
{
	fprintf(stderr, "Initial GPS...\n");
	serial_initial("/dev/ttySAC2", &GpsDevice, 9600);
}

void GPS_Read(GpsInfo *pGPS)
{
	
	while(1){
		memset((char *)Gps_str, 0, 512);
		len = serial_read(&GpsDevice, Gps_str, 512);
		
		if(len>0){

			if(strstr(Gps_str,"$GPRMC")!=NULL){
				nmea_parse_GPRMC(Gps_str, len, &vGPRMC);

				pGPS->status    = vGPRMC.status;
				pGPS->lat       = vGPRMC.lat;
				pGPS->ns        = vGPRMC.ns;
				pGPS->lon       = vGPRMC.lon;
				pGPS->ew        = vGPRMC.ew;
				pGPS->speed     = (vGPRMC.speed * NMEA_TUD_KNOTS);
				pGPS->direction = vGPRMC.direction;
				
				break;
			}
		}
	}
}

void GPS_InfoPrintf(GpsInfo *pGPS)
{
	fprintf(stderr, "\n\n========================\n");
	fprintf(stderr, "Status    : %s\n",     (pGPS->status=='A')?("Active"):("Inactive"));
	fprintf(stderr, "Latitude  : %5.4f\n",   pGPS->lat);
	fprintf(stderr, "Longitude : %5.4f\n",   pGPS->lon);
	fprintf(stderr, "Speed     : %f Km/h\n", pGPS->speed);
	fprintf(stderr, "direction : %f\n",      pGPS->direction);
	fprintf(stderr, "========================\n\n");
}

void GPS_Close(void)
{
	serial_close(&GpsDevice);	
}
