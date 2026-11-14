#pragma once


#define KO_0  0
#define KO_1  1
#define KO_2  2
#define KO_3  3
#define KO_4  4
#define KO_5  5
#define KO_6  6
#define KO_7  7
#define KO_8  8
#define KO_9  9

#define KO_Teachin  100

//direct answer after msg recive & TEACHin
#define msg_A5_20_01 1
#define msg_A5_20_04 2
#define msg_A5_20_06 3
#define TEACHIN_A52001 10
#define TEACHIN_A52004 11
#define TEACHIN_A52006 12



#define RockerA  1
#define RockerB  2
#define RockerC  3
#define RockerD  4

#define Wippen1  1
#define Wippen2  2
#define Wippen3  3
#define Wippen4  4

// VLD Profils
#define D2_00    0
#define D2_01    1
#define D2_02    2
#define D2_03    3
#define D2_04    4
#define D2_05    5
#define D2_06    6
#define D2_07    7
#define D2_0A    10
#define D2_10    16
#define D2_11    17
#define D2_14    20
#define D2_20    32
#define D2_30    48
#define D2_31    49
#define D2_32    50
#define D2_33    51
#define D2_34    52
#define D2_40    64
#define D2_50    80
#define D2_A0    160
#define D2_B0    176

// 4BS Profils
#define A5_02    2 
#define A5_04    4 
#define A5_05    5
#define A5_06    6 
#define A5_07    7 
#define A5_08    8 
#define A5_09    9 
#define A5_10    16 
#define A5_11    17
#define A5_12    18 
#define A5_13    19 
#define A5_14    20 
#define A5_20    32 
#define A5_30    48
#define A5_37    55 
#define A5_38    56
#define A5_3F    63
#define A5_special 255


// Profilnummern der einzelnen EEP-Profile (A5_02_01, D2_01_12, F6_10_00, ...) = Werte der Profilauswahl in der ETS.
// Sie werden vor jedem Build von scripts/generate_profiles.py aus src/EnoceanModule.share.xml erzeugt.
#include "EnoceanProfilsGenerated.h"

// Sonderfälle, die sich nicht aus dem Profiltext ableiten lassen:
// A5-38-08 Zentralbefehl (Gateway -> Aktor), Auswahl Schalt- bzw. Dimmaktor (ParamENO_CHProfil4BS38)
#define A5_38_08_SWITCH 410
#define A5_38_08_DIM 411
// spezielles 4BS-Profil eines D5-00-01 Fensterkontaktes (Profil4BSAdd)
#define D5_00_01_V2 401

// RPS-Profile, die in der ETS (noch) nicht auswählbar sind
#define F6_01_01 282
#define F6_02_01 283
#define F6_02_02 284
#define F6_02_04 286
#define F6_03_01 287
#define F6_03_02 288
