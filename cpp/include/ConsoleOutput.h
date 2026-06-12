/**
 * @file ConsoleOutput.h
 * @brief Global toggle for routine console chatter.
 */

#pragma once

namespace btls
{
	/**
	 * @brief Enables routine progress messages (buffer-flush notices).
	 *
	 * Off by default: a library should be quiet, and with parallel
	 * workers the interleaved messages are unreadable anyway. Errors
	 * and warnings ("***" messages) are always printed regardless.
	 * Toggle from Python via ``libbtls.set_console_output(True)``.
	 */
	inline bool console_output = false;
}
