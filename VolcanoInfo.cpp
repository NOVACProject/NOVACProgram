#include "StdAfx.h"
#include "volcanoinfo.h"

// The global database of volcanoes
CVolcanoInfo g_volcanoes;

CVolcanoInfo::CVolcanoInfo(void)
{
	// Setting up the information about the volcanoes
	int index = 0;

	m_name[index].Format("Arenal");
	m_simpleName[index].Format("arenal");
	m_peakLatitude[index]		= 10.46;
	m_peakLongitude[index]		= -84.7;
	m_peakHeight[index]			= 1670;
	m_hoursToGMT[index]			= -6; // Costa Rica
	m_observatory[index]		= 11;

	++index;
	m_name[index].Format("Poás");
	m_simpleName[index].Format("poas");
	m_peakLatitude[index]		= 10.20;
	m_peakLongitude[index]		= -84.23;
	m_peakHeight[index]			= 2708;
	m_hoursToGMT[index]			= -6;	// Costa Rica
	m_observatory[index]		= 12;

	++index;
	m_name[index].Format("Turrialba");
	m_simpleName[index].Format("turrialba");
	m_peakLatitude[index]		= 10.025;
	m_peakLongitude[index]		= -83.767;
	m_peakHeight[index]			= 3340;
	m_hoursToGMT[index]			= -6;	// Costa Rica
	m_observatory[index]		= 12;

	++index;
	m_name[index].Format("Santa Ana");
	m_simpleName[index].Format("santa_ana");
	m_peakLatitude[index]		= 13.85;
	m_peakLongitude[index]		= -89.63;
	m_peakHeight[index]			= 2381;
	m_hoursToGMT[index]			= -6; // El Salvador
	m_observatory[index]		= 13;

	++index;
	m_name[index].Format("San Miguel");
	m_simpleName[index].Format("san_miguel");
	m_peakLatitude[index]		= 13.43;
	m_peakLongitude[index]		= -88.27;
	m_peakHeight[index]			= 2130;
	m_hoursToGMT[index]			= -6; // El Salvador
	m_observatory[index]		= 13;

	++index;
	m_name[index].Format("Popocatépetl");
	m_simpleName[index].Format("popocatepetl");
	m_peakLatitude[index]		= 19.02;
	m_peakLongitude[index]		= -98.62;
	m_peakHeight[index]			= 5426;
	m_hoursToGMT[index]			= -6; // México
	m_observatory[index]		= 17;

	++index;
	m_name[index].Format("Fuego de Colima");
	m_simpleName[index].Format("fuego_de_colima");
	m_peakLatitude[index]		= 19.51;
	m_peakLongitude[index]		= -103.62;
	m_peakHeight[index]			= 3850;
	m_hoursToGMT[index]			= -6; // México
	m_observatory[index]		= 17;

	++index;
	m_name[index].Format("San Cristóbal");
	m_simpleName[index].Format("san_cristobal");
	m_peakLatitude[index]		= 12.70;
	m_peakLongitude[index]		= -87.0;
	m_peakHeight[index]			= 1745;
	m_hoursToGMT[index]			= -6;	// Nicaragua
	m_observatory[index]		= 4;

	++index;
	m_name[index].Format("Masaya");
	m_simpleName[index].Format("masaya");
	m_peakLatitude[index]		= 11.98;
	m_peakLongitude[index]		= -86.16;
	m_peakHeight[index]			= 635;
	m_hoursToGMT[index]			= -6;	// Nicaragua
	m_observatory[index]		= 4;

	++index;
	m_name[index].Format("Galeras");
	m_simpleName[index].Format("galeras");
	m_peakLatitude[index]		= 1.22;
	m_peakLongitude[index]		= -77.37;
	m_peakHeight[index]			= 4276;
	m_hoursToGMT[index]			= -5; // Colombia
	m_observatory[index]		= 5;

	++index;
	m_name[index].Format("Nevado del Ruiz");
	m_simpleName[index].Format("nevado_del_ruiz");
	m_peakLatitude[index]		= 4.89;
	m_peakLongitude[index]		= -75.32;
	m_peakHeight[index]			= 5321;
	m_hoursToGMT[index]			= -5; // Colombia
	m_observatory[index]		= 5;

	++index;
	m_name[index].Format("Nevado del Huila");
	m_simpleName[index].Format("nevado_del_huila");
	m_peakLatitude[index]		= 2.93;
	m_peakLongitude[index]		= -76.03;
	m_peakHeight[index]			= 5364 ;
	m_hoursToGMT[index]			= -5; // Colombia
	m_observatory[index]		= 5;

	++index;
	m_name[index].Format("Nyiragongo");
	m_simpleName[index].Format("nyiragongo");
	m_peakLatitude[index]		= -1.516732; //-1.40762;
	m_peakLongitude[index]		= 29.24668;  //29.2091;
	m_peakHeight[index]			= 3470;
	m_hoursToGMT[index]			= +2; // Dem. Rep. Congo
	m_observatory[index]		= 11;

	++index;
	m_name[index].Format("Nyamuragira");
	m_simpleName[index].Format("nyamuragira");
	m_peakLatitude[index]		= -1.41;
	m_peakLongitude[index]		= 29.20;
	m_peakHeight[index]			= 3058;
	m_hoursToGMT[index]			= +2; // Dem. Rep. Congo
	m_observatory[index]		= 11;

	++index;
	m_name[index].Format("Etna");
	m_simpleName[index].Format("etna");
	m_peakLatitude[index]		= 37.752;
	m_peakLongitude[index]		= 14.995;
	m_peakHeight[index]			= 3300;
	m_hoursToGMT[index]			= +1; // Italy
	m_observatory[index]		= 6;

	++index;
	m_name[index].Format("La Soufrière");
	m_simpleName[index].Format("la_soufriere");
	m_peakLatitude[index]		= 16.05;
	m_peakLongitude[index]		= -61.67;
	m_peakHeight[index]			= 1467;
	m_hoursToGMT[index]			= 0; // France
	m_observatory[index]		= 9;

	++index;
	m_name[index].Format("Tungurahua");
	m_simpleName[index].Format("tungurahua");
	m_peakLatitude[index]		= -1.467;
	m_peakLongitude[index]		= -78.442;
	m_peakHeight[index]			= 5023;
	m_hoursToGMT[index]			= -5; // Equador
	m_observatory[index]		= 3;

	++index;
	m_name[index].Format("Cotopaxi");
	m_simpleName[index].Format("cotopaxi");
	m_peakLatitude[index]		= -0.677;
	m_peakLongitude[index]		= -78.436;
	m_peakHeight[index]			= 5911;
	m_hoursToGMT[index]			= -5;	// Equador
	m_observatory[index]		= 3;

	++index;
	m_name[index].Format("Pacaya");
	m_simpleName[index].Format("pacaya");
	m_peakLatitude[index]		= 14.381;
	m_peakLongitude[index]		= 90.601;
	m_peakHeight[index]			= 2552;
	m_hoursToGMT[index]			= -6; // Guatemala
	m_observatory[index]		= 8;

	++index;
	m_name[index].Format("Piton de la Fournaise");
	m_simpleName[index].Format("piton_de_la_fournaise");
	m_peakLatitude[index]		= -21.23;
	m_peakLongitude[index]		= 55.71;
	m_peakHeight[index]			= 2632;
	m_hoursToGMT[index]			= +4; // Reunion
	m_observatory[index]		= 9;

	++index;
	m_name[index].Format("Santiaguito");
	m_simpleName[index].Format("santiaguito");
	m_peakLatitude[index]		= 14.756;
	m_peakLongitude[index]		= 91.552;
	m_peakHeight[index]			= 3772;
	m_hoursToGMT[index]			= -6; // Guatemala
	m_observatory[index]		= 8;

	++index;
	m_name[index].Format("Fuego (Guatemala)");
	m_simpleName[index].Format("fuego_guatemala");
	m_peakLatitude[index]		= 14.473;
	m_peakLongitude[index]		= -90.880;
	m_peakHeight[index]			= 3763;
	m_hoursToGMT[index]			= -6; // Guatemala
	m_observatory[index]		= 8;

	++index;
	m_name[index].Format("Vulcano");
	m_simpleName[index].Format("vulcano");
	m_peakLatitude[index]		= 38.404;
	m_peakLongitude[index]		= 14.986;
	m_peakHeight[index]			= 500;
	m_hoursToGMT[index]			= +1; // Italy
	m_observatory[index]		= 7;

	++index;
	m_name[index].Format("Stromboli");
	m_simpleName[index].Format("stromboli");
	m_peakLatitude[index]		= 38.789;
	m_peakLongitude[index]		=  15.213;
	m_peakHeight[index]			= 924;
	m_hoursToGMT[index]			= +1; // Italy
	m_observatory[index]		= 7;
	

	++index;
	m_name[index].Format("Yasur");
	m_simpleName[index].Format("yasur");
	m_peakLatitude[index]		= -19.53;
	m_peakLongitude[index]		= 169.442;
	m_peakHeight[index]			= 361;
	m_hoursToGMT[index]			= +11; // ??? Indonesia
	m_observatory[index]		= 2;


	++index;
	m_name[index].Format("Villarica");
	m_simpleName[index].Format("villarica");
	m_peakLatitude[index]		= -39.42;
	m_peakLongitude[index]		= -71.93;
	m_peakHeight[index]			= 2847;
	m_hoursToGMT[index]			= -7; // Chile
	m_observatory[index]		= 2;

	++index;
	m_name[index].Format("Chalmers");
	m_simpleName[index].Format("chalmers");
	m_peakLatitude[index]		= 0.0;
	m_peakLongitude[index]		= 0.0;
	m_peakHeight[index]			= 0;
	m_hoursToGMT[index]			= +1;
	m_observatory[index]		= 1;

	++index;
	m_name[index].Format("Harestua");
	m_simpleName[index].Format("harestua");
	m_peakLatitude[index]		= 60.21;
	m_peakLongitude[index]		= 10.75;
	m_peakHeight[index]			= 600;
	m_hoursToGMT[index]			= +1;
	m_observatory[index]		= 1;

	m_volcanoNum				= index+1;

	m_preConfiguredVolcanoNum	= m_volcanoNum;
}

CVolcanoInfo::~CVolcanoInfo(void)
{
}
