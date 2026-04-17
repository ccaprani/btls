/**
 * @file VehicleClassification.h
 * @brief Interface for vehicle classification schemes — axle-count and pattern-based.
 */

#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <memory>
#include <algorithm>

class CVehicle; // pre-defined
typedef std::shared_ptr<CVehicle> CVehicle_sp;

/**
 * @brief Selector for the classification system in use.
 */
enum EClassificationSystem
{
	eAxleCount = 0,  ///< Classify by number of axles.
	ePattern         ///< Classify by axle-spacing pattern.
};

/**
 * @brief A vehicle classification label: ID, pattern string, and description.
 */
struct Classification
{
	/**
	 * @brief Construct with an ID and description.
	 */
	Classification(size_t indx, std::string desc)
	{
		m_ID = indx;
		m_String = "";
		m_Desc = desc;
	}

	/**
	 * @brief Construct with an ID, pattern string, and description.
	 */
	Classification(size_t indx, std::string str, std::string desc)
	{
		m_ID = indx;
		m_String = str;
		m_Desc = desc;
	}

	size_t m_ID;          ///< Numeric class identifier.
	std::string m_String; ///< Pattern string (for pattern-based schemes).
	std::string m_Desc;   ///< Human-readable class description.

	/// @brief Equality by (ID, String, Desc) tuple.
	inline bool operator==(const Classification& other) const
	{
		return tie() == other.tie();
	};

	/// @brief Tie fields into a comparable tuple.
	std::tuple<size_t, std::string, std::string> tie() const
	{ return std::tie(m_ID, m_String, m_Desc); }
};

/**
 * @brief Abstract base for vehicle classification schemes.
 *
 * CVehicleClassification maintains a list of known classes and exposes
 * a pure-virtual setClassification() that concrete subclasses override
 * to tag each vehicle with the appropriate class based on its
 * properties.
 *
 * Two schemes are provided:
 * - @ref CVehClassAxle — classifies by number of axles (2/3/4/5).
 * - @ref CVehClassPattern — classifies by axle-spacing pattern.
 *
 * @see CVehicle
 */
class CVehicleClassification
{
public:
	CVehicleClassification();
	virtual ~CVehicleClassification();

	/**
	 * @brief Tag @p pVeh with a class from this scheme.
	 */
	virtual void setClassification(CVehicle_sp pVeh) = 0;

	/// @brief Get the total number of known classes.
	size_t getNoClasses() { return m_nClasses; };

	/**
	 * @brief Get the i-th class.
	 * @param[in] i Zero-based class index.
	 */
	Classification getClass(size_t i);

	/**
	 * @brief Look up the numeric ID for a classification label.
	 */
	size_t getClassID(Classification cl);

protected:
	template <typename T>
	std::string to_string(T const& value)
	{
		std::stringstream sstr;
		sstr << value;
		return sstr.str();
	}

	EClassificationSystem m_System;              ///< Active classification system.
	std::vector<Classification> m_vClasses;      ///< Known classes.
	size_t m_nClasses;                           ///< Number of classes.
	Classification m_DefaultClass;               ///< Default class (ID 0, "car").
	Classification m_CarClass;                   ///< Dedicated car class.
};
typedef std::shared_ptr<CVehicleClassification> CVehicleClassification_sp;  ///< Shared-pointer alias.

/**
 * @brief Classifies vehicles by number of axles (2, 3, 4, or 5).
 */
class CVehClassAxle : public CVehicleClassification
{
public:
	CVehClassAxle();
	~CVehClassAxle(){};

	/// @brief Tag @p pVeh by its axle count.
	void setClassification(CVehicle_sp pVeh) override;

private:
	Classification m_2axle;  ///< 2-axle class.
	Classification m_3axle;  ///< 3-axle class.
	Classification m_4axle;  ///< 4-axle class.
	Classification m_5axle;  ///< 5-axle class.
};
typedef std::shared_ptr<CVehClassAxle> CVehClassAxle_sp;  ///< Shared-pointer alias.

/**
 * @brief Classifies vehicles by their axle-spacing pattern string.
 */
class CVehClassPattern : public CVehicleClassification
{
public:
	CVehClassPattern();
	~CVehClassPattern(){};

	/// @brief Tag @p pVeh by its derived spacing-pattern string.
	void setClassification(CVehicle_sp pVeh) override;

private:
	/**
	 * @brief Derive the pattern string for @p pVeh from its axle spacings.
	 */
	std::string getPattern(CVehicle_sp pVeh);

	std::vector<std::string> m_vPatternStrings;  ///< Known pattern strings.
};
typedef std::shared_ptr<CVehClassPattern> CVehClassPattern_sp;  ///< Shared-pointer alias.
