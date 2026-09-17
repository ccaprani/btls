/**
 * @file FilePath.h
 * @brief Output-path helper.
 */

#pragma once

#include <string>

namespace btls
{
	/**
	 * @brief Join an output directory and a file name.
	 *
	 * An empty directory keeps the historical behaviour of writing into
	 * the current working directory.
	 */
	inline std::string outPath(const std::string& dir, const std::string& name)
	{
		return dir.empty() ? name : dir + "/" + name;
	}
}
