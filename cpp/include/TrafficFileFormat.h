#pragma once

#include <stdexcept>
#include <string>

/** @brief Traffic file formats supported by BTLS/PyBTLS. */
enum class ETrafficFileFormat
{
	Castor = 1,
	Bedit = 2,
	Ditis = 3,
	Mon = 4,
	Siwim = 5
};

/** @brief Physical/text structure of a traffic file format. */
enum class ETrafficFileKind
{
	FixedWidth,
	HeaderCsv
};

/** @brief Format metadata and capability flags. */
struct TrafficFileFormatSpec
{
	ETrafficFileFormat Format;
	ETrafficFileKind Kind;
	const char* Name;
	const char* Delimiter;
	bool CanRead;
	bool CanWrite;
};

inline TrafficFileFormatSpec getTrafficFileFormatSpec(size_t filetype)
{
	switch (filetype)
	{
	case 1: return { ETrafficFileFormat::Castor, ETrafficFileKind::FixedWidth, "CASTOR", ",", true, true };
	case 2: return { ETrafficFileFormat::Bedit, ETrafficFileKind::FixedWidth, "BEDIT", ",", true, true };
	case 3: return { ETrafficFileFormat::Ditis, ETrafficFileKind::FixedWidth, "DITIS", ",", true, true };
	case 4: return { ETrafficFileFormat::Mon, ETrafficFileKind::FixedWidth, "MON", ",", true, true };
	case 5: return { ETrafficFileFormat::Siwim, ETrafficFileKind::HeaderCsv, "SIWIM", ",", true, false };
	default:
		throw std::invalid_argument("Unsupported traffic file format: " + std::to_string(filetype));
	}
}

inline TrafficFileFormatSpec requireTrafficFileReadFormat(size_t filetype)
{
	TrafficFileFormatSpec spec = getTrafficFileFormatSpec(filetype);
	if (!spec.CanRead)
		throw std::invalid_argument(std::string("Traffic file format ") + spec.Name + " does not support reading");
	return spec;
}

inline TrafficFileFormatSpec requireTrafficFileWriteFormat(size_t filetype)
{
	TrafficFileFormatSpec spec = getTrafficFileFormatSpec(filetype);
	if (!spec.CanWrite)
		throw std::invalid_argument(std::string("Traffic file format ") + spec.Name + " does not support writing");
	return spec;
}
