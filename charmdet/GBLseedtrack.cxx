/*
 * GBLseedtrack.cpp
 *
 *  Created on: 29.04.2020
 *      Author: bieschke
 *  Modified on: 12.06.2020
 *      Author: Daniel Bick
 */

#include "GBLseedtrack.h"

using std::vector;
using std::pair;

/**
 * Constructor building a @c GBL_seed_track object from an already fitted @c genfit::Tarck object.
 * This way, a track that was already fitted by the genfit fitter can be reproduced with the GBL algorithm. The hits
 * as well as the vertex position and momentum vector are extracted from the genfit track and converted to the format
 * used internally in the @c GBL_seed_track. From the constructed object, the extracted and converted data can be
 * viewed by using the getter methods get_hits(), get_position() and so on.
 *
 * @brief Constructor using a genfit::Track as seed
 *
 * @author Stefan Bieschke
 * @date Nov. 11, 2020
 * @version 1.0
 *
 * @param track An already fitted Track from the genfit pacakge that is used as seed for the GBL algorithm
 */
GBL_seed_track::GBL_seed_track(const genfit::Track& track)
{
	vector<genfit::TrackPoint*> points = track.getPoints();
	size_t n_points = points.size();

	m_hits.resize(n_points);

	for(size_t i = 0; i < n_points; ++i)
	{
		genfit::TrackPoint* point = points[i];
		genfit::AbsMeasurement* raw_measurement = point->getRawMeasurement();
		int det_id = raw_measurement->getDetId();
		double drift_radius = raw_measurement->getRawHitCoords()[6];
		pair<int,double> hit_info = std::make_pair(det_id, drift_radius);

		m_hits[i] = hit_info;
	}

	m_position = track.getFittedState().getPos();
	m_direction = track.getFittedState().getMom();
}


/**
 * Constructor building a @c GBL_seed_track object from a simple, straight track model, which is described by a position on
 * the track and a momentum (direction) vector. Both of these are provided as three-dimensional vectors, wrapped into a
 * std::vector<TVector3> of length two. Additionally, a list of hits is needed that were recorded for this particular track.
 *
 * These values can for example be the result of a pattern recognition. They could as well be extracted from the MC truth
 * when using simulated data.
 *
 * @brief Constructor using a momentum and vertex position vectors as well as a list of hits
 *
 * @author Stefan Bieschke
 * @date Nov. 11, 2020
 * @version 1.0
 *
 * @param pos_mom vector of length two, containing a 3D-vector pointing to a position on the track, for example the
 * vertex position on its zero-index and a 3D vector holding its direction (e.g momentum vector) in its one-index.
 *
 * @param hits list of hits that were recorded for this track. The individual hits are provided as pairs of int and double
 * numbers, the int being the detector ID of the hit tube and the double being the drift distance in a length unit (default is cm).
 * This list of hits must be ordered by distance of the hit tube from the target (in the setup used for muflux measurement
 * this is the tube's z-coordinate) in ascending order.
 */
GBL_seed_track::GBL_seed_track(const vector<TVector3>& pos_mom, const vector<pair<int,double>> hits)
{
	m_position = pos_mom[0];
	m_direction = pos_mom[1];
	m_hits.resize(hits.size());
	//TODO copy ctor invokation
	for(size_t i = 0; i < m_hits.size(); ++i)
	{
		m_hits[i] = hits[i];
	}
}

/*
 * Constructor for seed track from pattern reco. Creates seedrack without hits yet, only momentum and direction is provided. Hits should then be added by add_hit(). 
 *
 * @brief Constructor
 *
 * @param position A point on the track
 * @param direction Direction of the track
 *
 * @author Daniel Bick
 * @date 12.06.2020
 * @version 1.0
 * 
 */
GBL_seed_track::GBL_seed_track(TVector3 position, TVector3 direction)
{
	m_position = position;
	m_direction = direction;
}

GBL_seed_track::GBL_seed_track::~GBL_seed_track()
{
	// TODO Auto-generated destructor stub
}

const size_t GBL_seed_track::get_number_of_hits() const
{
	return m_hits.size();
}


const TVector3& GBL_seed_track::get_position() const
{
	return m_position;
}

const TVector3& GBL_seed_track::get_direction() const
{
	return m_direction;
}

const vector<pair<int,double>>& GBL_seed_track::get_hits() const
{
	return m_hits;
}



const vector<int> GBL_seed_track::get_hit_detIDs() const
{
	vector<int> result(m_hits.size());

	for(size_t i = 0; i < result.size(); ++i)
	{
		result[i] = m_hits[i].first;
	}

	return result;
}

/*
 * When the seed track is created with direcion and momentum only, hits can be added from the pattern reco
 *
 * @brief Adding hits from pattern reco
 *
 * @param detectorID The detector ID of the hit
 * @param driftradius The drift radius of the hit
 *
 * @author Daniel Bick
 * @date 12.06.2020
 * @version 1.0
 *
 */
void GBL_seed_track::add_hit(int detectorID, double driftradius)
{
	std::pair<int, double> hit(detectorID, driftradius);
	m_hits.push_back(hit);
}
