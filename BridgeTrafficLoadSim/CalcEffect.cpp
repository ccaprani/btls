// CalcEffect.cpp: implementation of the CCalcEffect class.
//
//////////////////////////////////////////////////////////////////////

#include "CalcEffect.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCalcEffect::CCalcEffect()
{

}

CCalcEffect::CCalcEffect(double span)
{
	m_span = span;
}

CCalcEffect::~CCalcEffect()
{

}

void CCalcEffect::setSpan(double span)
{
	m_span = span;
}

double CCalcEffect::giveEffect(int j, double Xpos, double AxleWeight, int lane = 0)
{
	double seffect = 0.0;
	double ordinate = 0.0;

	//this account for the bridge time origin begin at bridge RHS
	//while the IL expressions origin is at bridge LHS
	Xpos = m_span - Xpos;

	switch(j)
	{
	case 1:
		{
			ordinate = give_effect1(Xpos);	// BM centre of SS bridge
			break;
		}
	case 2:
		{
			ordinate = give_effect2(Xpos);	// BM central support-two span
			break;
		}
	case 3:
		{
			ordinate = give_effect3(Xpos);	// SF LHS SS bridge
			break;
		}
	case 4:
		{
			ordinate = give_effect4(Xpos);	// SF RHS SS bridge
			break;
		}
	case 5:
		{
			ordinate = give_effect5(Xpos);	// SF LHS 2-span
			break;
		}
	case 6:
		{
			ordinate = give_effect6(Xpos);	// SF RHS 2-span
			break;
		}
	case 7:
		{
			ordinate = 1.0;	// Uniform IL for total GVW on bridge
			break;
		}

//****************************************************************
//****** THE FOLLOWING IS FOR THE SHANGHAI CONFERENCE ONLY *******
//****************************************************************
// j is the effect number - Girder A = 8, Girder B = 9,  Girder C = 10
// Girder A - Lane 2 - Ch 7 & 10, Girder B - Lane 2 - Ch 11 & 12, Girder C - Lane 1 - Ch 8 & 9

	case 8:
		{
			if(lane == 1)
				ordinate = give_effectA1(Xpos);
			else if(lane == 2)
				ordinate = give_effectA2(Xpos);
			else
				ordinate = 0;
			break;
		}
	case 9:
		{
			if(lane == 1)
				ordinate = give_effectB1(Xpos);
			else if(lane == 2)
				ordinate = give_effectB2(Xpos);
			else
				ordinate = 0;
			break;
		}
	case 10:
		{
			if(lane == 1)
				ordinate = give_effectC1(Xpos);
			else if(lane == 2)
				ordinate = give_effectC2(Xpos);
			else
				ordinate = 0;
			break;
		}

//****************************************************************
//******** THE FOLLOWING IS FOR THE DYNAMICS PAPER ONLY **********
//****************************************************************
// When the truck is in lane 1, beam 1 has the same effect as 
// beam 5 when the truck is in lane 2 for example. this is why
// the lane2 case returns a different beam number

	case 11: // Beam 1 as per diagram of the paper
		{
			if(lane == 1)
				ordinate = give_Beam1(Xpos);
			else if(lane == 2)
				ordinate = 1.1974*give_Beam5(Xpos);
			else
				ordinate = 0;
			break;
		}
	case 12: // Beam 2 as per diagram of the paper
		{
			if(lane == 1)
				ordinate = give_Beam2(Xpos);
			else if(lane == 2)
				ordinate = 1.0516*give_Beam4(Xpos);
			else
				ordinate = 0;
			break;
		}
	case 13: // Beam 3 as per diagram of the paper
		{
			if(lane == 1)
				ordinate = give_Beam3(Xpos);
			else if(lane == 2)
				ordinate = 1.0159*give_Beam3(Xpos);
			else
				ordinate = 0;
			break;
		}
	case 14: // Beam 4 as per diagram of the paper
		{
			if(lane == 1)
				ordinate = give_Beam4(Xpos);
			else if(lane == 2)
				ordinate = 0.9692*give_Beam2(Xpos);
			else
				ordinate = 0;
			break;
		}
	case 15: // Beam 5 as per diagram of the paper
		{
			if(lane == 1)
				ordinate = give_Beam5(Xpos);
			else if(lane == 2)
				ordinate = 0.9624*give_Beam1(Xpos);
			else
				ordinate = 0;
			break;
		}

	case 16:
		{
			ordinate = give_effect16(Xpos);	// BM B224 in Vienna
			break;
		}

	}

	// to change IL's from 1 tonne to 1 kN
	seffect= AxleWeight * ordinate;
	return seffect;
}

double CCalcEffect::give_effect1(double x)
{
	double L = m_span;
	double s = L/2;
	double ordinate = 0.0;
	
	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = x * (1 - s/L);
		else
			ordinate = s * (1 - x/L);
	}
	return ordinate;
}

double CCalcEffect::give_effect2(double x)
{
	double L = m_span;
	double s = L/2;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = x * (s*s - x*x) / (4*s*s);   
		else
		{
			double y = 2 * s - x;
			ordinate = y * (s*s - y*y) / (4*s*s);
		}
	}
	return ordinate;
}

double CCalcEffect::give_effect3(double x)
{
	double L = m_span;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		ordinate = 1 - x / L;
	}
	return ordinate;
}

double CCalcEffect::give_effect4(double x)
{
	double L = m_span;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		ordinate = x / L;
	}
	return ordinate;
}

double CCalcEffect::give_effect5(double x)
{
	double L = m_span;
	double s = L/2;
	double ordinate = 0.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = (1/s) * (-x * (s*s - x*x) / (4*s*s) + s - x);
		else
		{
			double y = 2 * s - x;
			ordinate = (1/s) * (-y * (s*s - y*y) / (4*s*s) );
		}
	}
	return ordinate;
}

double CCalcEffect::give_effect6(double x)
{
	double L = m_span;
	double s = L/2;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = (1/s) * (-x * (s*s - x*x) / (4*s*s) );
		else
		{
			double y = 2 * s - x;
			ordinate = (1/s) * (-y * (s*s - y*y) / (4*s*s) + s - y);
		}
	}
	return ordinate;
}

double CCalcEffect::give_effect16(double x)
{
	// This is for the 18.0 m bridge of the B224 in Vienna
	double L = m_span;
	double s = L/2;
	double ordinate = 0.0;
	double y = 0.0;
	double scaleFactor = 3.5;

	if(L != 18.0)
		return 1.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if((x >= 0.0) && (x < 5.8))
			y = -0.0158*x*x + 0.0252*x - 0.0036;
		if((x >= 5.8) && (x < 9.3))
			y = -0.1815*x + 0.6831;
		if((x >= 9.3) && (x < 12.5))
			y = 0.0525*x*x - 1.0162*x + 3.9178;
		if((x >= 12.5) && (x < 14.5))
			y = -0.0277*x*x + 0.9346*x - 7.9383;
		if((x >= 14.5) && (x < 16.9))
			y = 0.0825*x - 1.3994;
		if((x >= 16.9) && (x < 17.4))
			y = 0.0162*x - 0.2819;
		if((x >= 17.4) && (x <= 18.0))
			y = 0.001*x - 0.0179;
	}
	ordinate = -y * scaleFactor;
	return ordinate;
}

double CCalcEffect::give_effectA1(double x)
{
	// This is for the 18.0 m bridge of the B224 in Vienna
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;
	double scaleFactor = 0.075/9.81;

	if(L != 15.75)
		return 1.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if((x >= 0.0) && (x < 1.05))
			y = 5.8347*x*x*x - 9.0782*x*x + 4.8301*x + 0.6357;
		if((x >= 1.05) && (x < 2.4))
			y = 2.0511*x*x*x - 10.792*x*x + 19.967*x - 9.1463;
		if((x >= 2.4) && (x < 4.55))
			y = 0.6856*x*x*x - 7.1203*x*x + 26.448*x - 26.986;
		if((x >= 4.55) && (x < 5.0))
			y = 35.719*x*x*x - 515.75*x*x + 2480.9*x - 3964.865;
		if((x >= 5.0) && (x < 5.4))
			y = 14.019*x*x*x - 231.04*x*x + 1264*x - 2285.75;
		if((x >= 5.4) && (x < 6.25))
			y = -23.266*x*x*x + 399.27*x*x - 2276.3*x + 4322.9;
		if((x >= 6.25) && (x < 7.45))
			y = 5.287*x*x*x	- 108.23*x*x + 738.56*x - 1667.1;
		if((x >= 7.45) && (x < 8.4))
			y = 0.2218*x*x*x - 5.6814*x*x + 47.382*x - 115.03;
		if((x >= 8.4) && (x < 8.9))
			y = -25.736*x*x*x + 672.08*x*x - 5847.6*x + 16965.45;
		if((x >= 8.9) && (x < 10.0))
			y = 1.9495*x*x*x - 54.989*x*x + 515.48*x - 1592.2;
		if((x >= 10.0) && (x < 10.7))
			y = 8.6474*x*x*x - 271.97*x*x + 2847.9*x - 9916.4;
		if((x >= 10.7) && (x < 11.1))
			y = -25.832*x*x*x + 849.01*x*x - 9299.6*x + 33959.57;
		if((x >= 11.1) && (x < 12.65))
			y = -1.1464*x*x*x + 40.934*x*x - 489.35*x + 1968.2;
		if((x >= 12.65) && (x < 13.4))
			y = 7.6583*x*x*x - 303.23*x*x + 3997.2*x - 17535.9;
		if((x >= 13.4) && (x < 15.15))
			y = 1.2936*x*x*x - 55.652*x*x + 795.15*x - 3769.6;
		if((x >= 15.15) && (x <= 15.75))
			y = 4.9127*x*x*x - 228.8*x*x + 3548.9*x - 18332.18;
	}
	ordinate = y * scaleFactor;
	return ordinate;
}

double CCalcEffect::give_effectA2(double x)
{
	// This is for the 18.0 m bridge of the B224 in Vienna
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;
	double scaleFactor = 0.075/9.81;

	if(L != 15.75)
		return 1.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if((x >= 0.0) && (x < 6.25))
			y = -0.14429*x*x*x + 1.8502*x*x + 0.52427*x + 0.98071;
		if((x >= 6.25) && (x < 7.5))
			y = 10.148*x*x*x - 212.995*x*x + 1494.7*x - 3458.3;
		if((x >= 7.5) && (x < 7.8))
			y = - 27.433*x*x + 423.38*x - 1580.6;
		if((x >= 7.8) && (x < 8.55))
			y = 11.667*x*x*x - 292.3*x*x + 2435.3*x - 6695.5;
		if((x >= 8.55) && (x < 9.65))
			y = - 7.862*x*x + 136.71*x - 543.62;
		if((x >= 9.65) && (x < 14.15))
			y = 0.4847*x*x*x - 17.009*x*x + 188.46*x - 627.49;
		if((x >= 14.15) && (x <= 15.75))
			y = - 4.0158*x + 62.962;
	}
	ordinate = y * scaleFactor;
	return ordinate;
}

double CCalcEffect::give_effectB1(double x)
{
	// This is for the 18.0 m bridge of the B224 in Vienna
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;
	double scaleFactor = 0.075/9.81;

	if(L != 15.75)
		return 1.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if((x >= 0.0) && (x < 4.0))
			y =	0.0335*x*x*x + 0.2898*x*x + 2.9172*x + 0.2807;
		if((x >= 4.0) && (x < 5.05))
			y =	10.087*x*x*x - 131.56*x*x + 574.17*x - 818.6;
		if((x >= 5.05) && (x < 7.9))
			y =	-1.1839*x*x*x + 22.496*x*x - 136.16*x + 292.33;
		if((x >= 7.9) && (x < 8.5))
			y =	-6.1237*x*x*x + 142.46*x*x - 1100*x + 2855.6;
		if((x >= 8.5) && (x < 12.9))
			y =	0.0271*x*x*x - 1.4121*x*x + 13.986*x + 4.2831;
		if((x >= 12.9) && (x < 14.9))
			y =	-0.0816*x*x*x + 3.9171*x*x - 65.851*x + 381.47;
		if((x >= 14.9) && (x <= 15.75))
			y =	0;
	}
	ordinate = y * scaleFactor;
	return ordinate;
}

double CCalcEffect::give_effectB2(double x)
{
	// This is for the 18.0 m bridge of the B224 in Vienna
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;
	double scaleFactor = 0.075/9.81;

	if(L != 15.75)
		return 1.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if((x >= 0.0) && (x < 8.0))
			y = 0.3329*x*x*x - 1.6134*x*x + 2.1223*x - 0.2342;
		if((x >= 8.0) && (x < 9.55))
			y = -6.3001*x*x*x + 144.46*x*x - 1072.5*x + 2645.2;
		if((x >= 9.55) && (x < 14.35))
			y = 0.1092*x*x*x - 0.5883*x*x - 50.916*x + 535.08;
		if((x >= 14.35) && (x <= 15.75))
			y = 2.8804*x*x*x - 129.93*x*x + 1947.8*x - 9702.4;
	}
	ordinate = y * scaleFactor;
	return ordinate;
}

double CCalcEffect::give_effectC1(double x)
{
	// This is for the 18.0 m bridge of the B224 in Vienna
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;
	double scaleFactor = 0.075/9.81;

	if(L != 15.75)
		return 1.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if((x >= 0.0) && (x < 5.4))
			y = 0.0544*x*x*x - 0.0219*x*x + 0.1165*x + 0.1626;
		if((x >= 5.4) && (x < 7.95))
			y = 2.1984*x*x - 18.198*x + 43.45;
		if((x >= 7.95) && (x < 8.7))
			y = -7.2528*x*x + 128.86*x - 529.16;
		if((x >= 8.7) && (x < 9.45))
			y = 10.539*x*x*x - 293.79*x*x + 2720.1*x - 8325;
		if((x >= 9.45) && (x < 10.55))
			y = -11.338*x + 145.12;
		if((x >= 10.55) && (x < 12.9))
			y = -7.1944*x + 101.18;
		if((x >= 12.9) && (x <= 15.75))
			y = -3.0485*x + 47.405;
	}
	ordinate = y * scaleFactor;
	return ordinate;
}

double CCalcEffect::give_effectC2(double x)
{
	// This is for the 18.0 m bridge of the B224 in Vienna
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;
	double scaleFactor = 0.075/9.81;

	if(L != 15.75)
		return 1.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if((x >= 0.0) && (x < 2.05))
			y = -0.3204*x*x*x + 1.4442*x*x - 0.1673*x + 0.2838;
		if((x >= 2.05) && (x < 5.4))
			y = -0.1607*x*x*x + 1.9172*x*x - 2.9417*x + 2.7184;
		if((x >= 5.4) && (x < 7.45))
			y = 1.3292*x*x*x - 25.586*x*x + 169.7*x - 362.56;
		if((x >= 7.45) && (x < 8.8))
			y = 1.9789*x*x*x - 49.851*x*x + 419.93*x - 1148.5;
		if((x >= 8.8) && (x < 9.8))
			y = 6.6826*x*x*x - 190.43*x*x + 1802.1*x - 5630.8;
		if((x >= 9.8) && (x < 13.35))
			y = 0*x*x*x - 0.0863*x*x - 4.3304*x + 81.069;
		if((x >= 13.35) && (x <= 15.75))
			y = 0.1491*x*x*x - 6.2516*x*x + 83.753*x - 350.94;
	}
	ordinate = y * scaleFactor;
	return ordinate;
}

double CCalcEffect::give_Beam1(double x)
{
	// This is for the 32.0 m Slovenian bridge only
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;

	if(L != 32.0)
		return 1.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if((x > 0.0) && (x < 12.0))
			y = 0.057956484*x*x + 6.48046164*x + 0.131103101;
		if((x >= 12.0) && (x < 16.0))
			y =  - 1.222510483*x*x + 39.36620131*x - 210.0466581;
		if((x >= 16.0) && (x < 20.0))
			y =  - 0.024330551*x*x - 6.215751938*x + 212.5088368;
		if((x >= 20.0) && (x < 32.0))
			y = 0.06888267*x*x - 10.09611656*x + 252.7322673;

	}
	ordinate = y > 0 ? y : 0; // avoids error with the IL fit
	return ordinate;
}

double CCalcEffect::give_Beam2(double x)
{
	// This is for the 32.0 m Slovenian bridge only
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;

	if(L != 32.0)
		return 1.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if((x > 0.0) && (x < 12.0))
			y = 0.008411341*x*x + 5.441575047*x + 0.029094903;
		if((x >= 12.0) && (x < 16.0))
			y =  - 1.16568968*x*x + 38.19420442*x - 223.6966973;
		if((x >= 16.0) && (x < 20.0))
			y = 0.749386288*x*x - 34.60978938*x + 450.9255277;
		if((x >= 20.0) && (x < 32.0))
			y =  - 0.026430423*x*x - 3.504751247*x + 139.0948593;

	}
	ordinate = y > 0 ? y : 0; // avoids error with the IL fit
	return ordinate;
}

double CCalcEffect::give_Beam3(double x)
{
	// This is for the 32.0 m Slovenian bridge only
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;

	if(L != 32.0)
		return 1.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if((x > 0.0) && (x < 12.0))
			y = 0.00676122*x*x + 4.412921571*x + 0.051755991;
		if((x >= 12.0) && (x < 16.0))
			y =  - 0.758368527*x*x + 24.78323361*x - 134.1174107;
		if((x >= 16.0) && (x < 20.0))
			y = 0.180262056*x*x - 11.31351606*x + 203.1483618;
		if((x >= 20.0) && (x < 32.0))
			y =  - 0.005298256*x*x - 3.798817844*x + 127.0034224;

	}
	ordinate = y > 0 ? y : 0; // avoids error with the IL fit
	return ordinate;
}

double CCalcEffect::give_Beam4(double x)
{
	// This is for the 32.0 m Slovenian bridge only
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;

	if(L != 32.0)
		return 1.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if((x > 0.0) && (x < 12.0))
			y =  - 0.014304907*x*x + 3.657311286*x - 0.026367194;
		if((x >= 12.0) && (x < 16.0))
			y =  - 0.331335348*x*x + 10.82707102*x - 40.53275206;
		if((x >= 16.0) && (x < 20.0))
			y =  - 0.294425441*x*x + 8.666089694*x - 15.39643429;
		if((x >= 20.0) && (x < 32.0))
			y = 0.004922603*x*x - 3.599748857*x + 110.2034206;

	}
	ordinate = y > 0 ? y : 0; // avoids error with the IL fit
	return ordinate;
}

double CCalcEffect::give_Beam5(double x)
{
	// This is for the 32.0 m Slovenian bridge only
	double L = m_span;
	double ordinate = 0.0;
	double y = 0.0;

	if(L != 32.0)
		return 1.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if((x > 0.0) && (x < 12.0))
			y =  - 0.069032909*x*x + 2.975063384*x - 0.195348214;
		if((x >= 12.0) && (x < 16.0))
			y =  - 0.137974515*x*x + 4.400484434*x - 7.552879359;
		if((x >= 16.0) && (x < 20.0))
			y =  - 0.130599282*x*x + 3.993480446*x - 2.924849068;
		if((x >= 20.0) && (x < 32.0))
			y =  - 0.051314563*x*x + 0.585417392*x + 33.66404089;

	}
	ordinate = y > 0 ? y : 0; // avoids error with the IL fit
	return ordinate;
}