/**
 * @file BlockMaxEvent.h
 * @brief Interface for the CBlockMaxEvent class — one block's block-max events per vehicle count.
 */

#pragma once

#include <vector>
#include "Event.h"

/**
 * @brief Container holding one block's worth of block-max events, indexed
 *        by vehicle count.
 *
 * CBlockMaxManager tracks the maximum @ref CEvent seen so far within the
 * current time block, broken down by the number of vehicles contributing
 * to each event (1-truck events, 2-truck events, etc.). CBlockMaxEvent
 * is the underlying container: one slot per vehicle-count bucket, each
 * holding the running maximum CEvent for that bucket.
 *
 * The slot list is grown on demand via AddExtraEvents() when an event
 * with more vehicles than any previously seen arrives.
 *
 * @see CBlockMaxManager
 */
class CBlockMaxEvent
{
public:
	/**
	 * @brief Construct a block-max container for the given output format.
	 * @param[in] fileFormat File format tag used when writing contained events.
	 */
	CBlockMaxEvent(size_t fileFormat);
	virtual ~CBlockMaxEvent();

	/**
	 * @brief Grow the slot list to cover up to @p nVehs vehicles.
	 * @param[in] nVehs Number of vehicles the new top bucket should handle.
	 */
	void AddExtraEvents(size_t nVehs);

	/**
	 * @brief Update the running maximum for bucket @p iEvent with @p Ev.
	 *
	 * @param[in] iEvent Zero-based vehicle-count bucket index.
	 * @param[in] Ev     Candidate event. Replaces the current maximum if larger.
	 */
	void UpdateEvent(size_t iEvent, CEvent Ev);

	/**
	 * @brief Get a reference to the event in bucket @p iEv.
	 * @param[in] iEv Zero-based vehicle-count bucket index.
	 * @return Reference to the stored @ref CEvent.
	 */
	CEvent& getEvent(size_t iEv);

	/// @brief Reset the container to an empty state.
	void clear();

	/// @brief Set the block identifier (typically the block index).
	void setID(size_t ID);

	/// @brief Get the block identifier.
	size_t getID();

	/// @brief Get the number of buckets currently stored.
	size_t getSize();

	/// @brief Set the number of load effects tracked per bucket.
	void setNoLoadEffects(size_t nLE);

private:
	size_t m_NoLoadEffects;         ///< Number of load effects tracked per bucket.
	size_t m_NoEvents;              ///< Running count of events processed (for stats).
	size_t m_ID;                    ///< Block identifier.
	std::vector<CEvent>	m_vEvents;  ///< One CEvent per vehicle-count bucket.

	size_t FILE_FORMAT;             ///< Output file format tag.
};
