#include "VehicleClassification.h"

#include "Vehicle.h"

CVehicleClassification::CVehicleClassification() 
	: m_DefaultClass(Classification(99, "Default")),
	  m_CarClass(Classification(0, "Car"))
{
	// class 99 is a bucket
	m_vClasses.push_back(m_DefaultClass);
	// class 0 is always a car
	m_vClasses.push_back(m_CarClass);
}


CVehicleClassification::~CVehicleClassification()
{
}

Classification CVehicleClassification::getClass(size_t i)
{
	if (i < m_nClasses)
		return m_vClasses.at(i);
	else
		return m_DefaultClass;
}

size_t CVehicleClassification::getClassID(Classification cl)
{
	// facncy search within the vector to find the index of the matching element
	std::vector<Classification>::iterator it = std::find(m_vClasses.begin(), m_vClasses.end(), cl);

	if (it != m_vClasses.end())
		return std::distance(m_vClasses.begin(), it);
	else
		return 0; // default classification should be at zero

}

//////////// CVehClassAxle //////////////////

CVehClassAxle::CVehClassAxle() :
m_2axle(Classification(1, "2-axle")),
m_3axle(Classification(1, "3-axle")),
m_4axle(Classification(1, "4-axle")),
m_5axle(Classification(1, "5-axle"))
{
	m_System = eAxleCount;

	m_vClasses.push_back(m_2axle);
	m_vClasses.push_back(m_3axle);
	m_vClasses.push_back(m_4axle);
	m_vClasses.push_back(m_5axle);

	m_nClasses = m_vClasses.size();
}

void CVehClassAxle::setClassification(CVehicle* pVeh)
{
	size_t nAxles = pVeh->getNoAxles();

	// MAGIC NUMBER - CAR WEIGHT THRESHOLD
	if (nAxles == 2 && pVeh->getGVW() < 34.334)
		pVeh->setClass(m_CarClass);
	else
	{
		switch (nAxles)
		{
		case 2:
			pVeh->setClass(m_2axle); break;
		case 3:
			pVeh->setClass(m_3axle); break;
		case 4:
			pVeh->setClass(m_4axle); break;
		case 5:
			pVeh->setClass(m_5axle); break;
		default:
			pVeh->setClass(m_DefaultClass);
		}
	}
}

//////////// CVehClassPattern //////////////////

CVehClassPattern::CVehClassPattern()
{
	m_System = ePattern;

	m_vPatternStrings.push_back(std::string("11"));
	m_vPatternStrings.push_back(std::string("123"));
	m_vPatternStrings.push_back(std::string("12"));
	m_vPatternStrings.push_back(std::string("1233"));
	m_vPatternStrings.push_back(std::string("122"));
	m_vPatternStrings.push_back(std::string("112"));
	m_vPatternStrings.push_back(std::string("113"));

	for (auto&& i : m_vPatternStrings)
	{
		std::string desc;
		desc = "Pattern " + i;
		m_vClasses.push_back(Classification(m_vClasses.size() - 1, i, desc));
	}

	m_nClasses = m_vClasses.size();
}

void CVehClassPattern::setClassification(CVehicle* pVeh)
{
	size_t nAxles = pVeh->getNoAxles();

	// MAGIC NUMBER - CAR WEIGHT THRESHOLD
	if (nAxles == 2 && pVeh->getGVW() < 34.334)
		pVeh->setClass(m_CarClass);
	else
	{
		std::string str = getPattern(pVeh);
		bool bSuccess = 0;

		for (auto&& i : m_vClasses)
		{
			if (i.m_String == str)
			{
				pVeh->setClass(i);
				bSuccess = true;
				break;
			}
		}
		if (!bSuccess)
			pVeh->setClass(m_DefaultClass);
	}
}

std::string CVehClassPattern::getPattern(CVehicle* pVeh)
{
	size_t nAxles = pVeh->getNoAxles();
	double maxAS = 2.1; // MAGIC NUMBER - max axle spacing for a group
	size_t groupCount = 1; // first axle
	std::string sPattern = "";

	for(size_t i = 0; i < nAxles-1; ++i)
	{
		if (pVeh->getAS(i) < maxAS)
			groupCount++;
		else
		{
			sPattern += to_string(groupCount);
			groupCount = 1;
		}
	}
	sPattern += to_string(groupCount); // collect last one
	return sPattern;
}
