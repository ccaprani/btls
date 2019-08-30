#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <memory>

class CVehicle; // pre-defined
typedef std::shared_ptr<CVehicle> CVehicle_ptr;

enum EClassificationSystem
{
	eAxleCount = 0,
	ePattern
};

struct Classification
{
	Classification(size_t indx, std::string desc)
	{
		m_ID = indx;
		m_String = "";
		m_Desc = desc;
	}
	Classification(size_t indx, std::string str, std::string desc)
	{
		m_ID = indx;
		m_String = str;
		m_Desc = desc;
	}
	size_t m_ID;
	std::string m_String;
	std::string m_Desc;

	// Pretty awesome C++14 example https://stackoverflow.com/questions/5740310/no-operator-found-while-comparing-structs-in-c
	inline bool operator==(const Classification& other) const
	{
		return tie() == other.tie();
	};
	std::tuple<int, std::string, std::string> tie() const 
	{ return std::tie(m_ID, m_String, m_Desc); }
};

class CVehicleClassification
{
public:
	CVehicleClassification();
	virtual ~CVehicleClassification();

	virtual void setClassification(CVehicle_ptr pVeh) = 0;
	size_t getNoClasses() { return m_nClasses; };
	Classification getClass(size_t i);
	size_t getClassID(Classification cl);

protected:
	template <typename T>
	std::string to_string(T const& value)
	{
		std::stringstream sstr;
		sstr << value;
		return sstr.str();
	}

	EClassificationSystem m_System;
	std::vector<Classification> m_vClasses;
	size_t m_nClasses;
	Classification m_DefaultClass;
	Classification m_CarClass;
};
typedef std::shared_ptr<CVehicleClassification> CVehicleClassification_ptr;

class CVehClassAxle : public CVehicleClassification
{
public:
	CVehClassAxle();
	~CVehClassAxle(){};

	void setClassification(CVehicle_ptr pVeh);

private:
	Classification m_2axle;
	Classification m_3axle;
	Classification m_4axle;
	Classification m_5axle;
};
typedef std::shared_ptr<CVehClassAxle> CVehClassAxle_ptr;

class CVehClassPattern : public CVehicleClassification
{
public:
	CVehClassPattern();
	~CVehClassPattern(){};

	void setClassification(CVehicle_ptr pVeh);

private:
	std::string getPattern(CVehicle_ptr pVeh);

	std::vector<std::string> m_vPatternStrings;
};
typedef std::shared_ptr<CVehClassPattern> CVehClassPattern_ptr;
