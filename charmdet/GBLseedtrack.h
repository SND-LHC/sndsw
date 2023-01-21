/*
 * GBLseedtrack.h
 *
 *  Created on: 29.04.2020
 *      Author: bieschke
 */

#include "Track.h"
#include "TVector3.h"
#include <vector>
#ifndef CHARMDET_GBLSEEDTRACK_H_
#define CHARMDET_GBLSEEDTRACK_H_


/**
 * A representation of a sinmple track that is used by the GBL algorithm as a seed. The seed
 * is used as a model, of which the parameters (slope and offsets, e.g vertex position perpendicular to detector normal) are
 * optimized in order to find the fitted track.
 * The @c GBL_seed_track contains a list of hits (as @ std::vector<std::pair<int,double>>)that were recorded for this
 * particular track and an initial value of the vertex position and a momentum as three dimensional vectors each.
 *
 * The hits that are contained in the @c GBL_seed_track is a vector (used as ordered array container) of pairs, each of which
 * contains an integer number, the detector ID of the drift tube that was hit and a double precision number, holding the drift
 * distance in a length unit. Default length unit at the time of writing is cm.
 *
 * @brief Representation of a straight track used for seeding the GBL algorithm
 *
 * @author Stefan Bieschke
 * @date Nov. 11, 2020
 * @version 1.0
 */
class GBL_seed_track
{
public:
	GBL_seed_track(const genfit::Track& track);
	GBL_seed_track(const std::vector<TVector3>& pos_mom, const std::vector<std::pair<int,double>> hits);
	GBL_seed_track(TVector3 position, TVector3 direction);

	virtual ~GBL_seed_track();

	const size_t get_number_of_hits() const;
	const TVector3& get_position() const;
	const TVector3& get_direction() const;

	const std::vector<std::pair<int,double>>& get_hits() const;

	const std::vector<int> get_hit_detIDs() const;

	void add_hit(int detectorID, double driftradius);

	
private:
	TVector3 m_position;
	TVector3 m_direction;

	std::vector<std::pair<int,double>> m_hits;
};

#endif /* CHARMDET_GBLSEEDTRACK_H_ */
