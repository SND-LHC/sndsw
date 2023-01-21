#include "MillepedeCaller.h"

using namespace std;


/**
 * Constructor. Initializes the wrapper to the Mille class. All paramaters of this constructor
 * are passed to the constructor of the Mille class.
 *
 * @brief Constructor
 *
 * @author Stefan Bieschke
 * @date Apr 9, 2019
 * @version 1.0
 *
 * @param outFileName string containing the filename written by mille function.
 * @param asBinary Flag that states if file should be written as binary
 * @param writeZero Flag that states if zero values should be kept or not
 */
MillepedeCaller::MillepedeCaller(const char* out_file_name)
{
	m_mersenne_twister = mt19937(19937);
	m_gbl_mille_binary = new gbl::MilleBinary(out_file_name,true,2000);
	stringstream root_output;
	root_output << "datatree_" << out_file_name << ".root";
	m_output_file = new TFile(root_output.str().c_str(),"RECREATE");
	m_output_tree = create_output_tree();
	m_survey.Init();


	m_modules["T1U"] = {};
	m_modules["T2V"] = {};
	m_modules["T3aX"] = {};
	m_modules["T3bX"] = {};
	m_modules["T3cX"] = {};
	m_modules["T3dX"] = {};
	m_modules["T4aX"] = {};
	m_modules["T4bX"] = {};
	m_modules["T4cX"] = {};
	m_modules["T4dX"] = {};
	for(auto vec : m_modules)
	{
		vec.second.reserve(48);
	}

	//generate list of tube ids
	//T1 and T2
	string module_name;
	for (char station = 1; station < 3; ++station)
	{
		for (char view = 0; view < 2; ++view)
		{
			switch (station)
			{
			case 1:
				module_name = view == 0 ? "T1X" : "T1U";
				break;
			case 2:
				module_name = view == 0 ? "T2V" : "T2X";
				break;
			}
			for (char plane = 0; plane < 2; ++plane)
			{
				for (char layer = 0; layer < 2; ++layer)
				{
					for (char tube = 1; tube < 13; ++tube)
					{
						int id = station * 10000000 + view * 1000000 + plane * 100000 + layer * 10000 + 2000 + tube;
						m_tube_id_to_module[id] = module_name;
						m_modules[module_name].push_back(id);
					}
				}
			}
		}

	}

	//T3 and T4
	for (char station = 3; station < 5; ++station)
	{
		for (char plane = 0; plane < 2; ++plane)
		{
			for (char layer = 0; layer < 2; ++layer)
			{
				for (char tube = 1; tube < 49; ++tube)
				{
					stringstream module_key;
					module_key << "T" << (int)station;
					if(tube <= 12)
					{
						module_key << "dX";
					}
					else if(tube <= 24)
					{
						module_key << "cX";
					}
					else if(tube <= 36)
					{
						module_key << "bX";
					}
					else
					{
						module_key << "aX";
					}
					string key = module_key.str();
					int id = station*10000000+plane*100000+layer*10000+2000+tube;
					m_modules[key].push_back(id);
					m_tube_id_to_module[id] = key;
				}
			}
		}
	}
	for(pair<string,vector<int>> entry : m_modules)
	{
		m_nominal_module_centerpos[entry.first] = calc_module_centerpos(entry);
	}
}



/**
 * Default destructor. Tears down the object
 *
 * @brief Destructor
 */
MillepedeCaller::~MillepedeCaller()
{
	delete m_gbl_mille_binary;
	m_output_file->cd();
	m_output_tree->Write();
	m_output_file->Write();
	m_output_file->Close();
}


std::vector<gbl::GblPoint> MillepedeCaller::list_hits(const GBL_seed_track* track, const alignment_mode& mode, double sigma_spatial, map<int,double>* pede_corrections, TTree* tree)
{
	vector<TVector3> linear_model = {track->get_position(), track->get_direction()};
	vector<gbl::GblPoint> result = {};

	vector<pair<int,double>> points = track->get_hits();
	size_t n_points = points.size();
	result.reserve(n_points);

	//sort hits by arc length
//	sort(points.begin(),points.end(),[](const genfit::TrackPoint* p1, const genfit::TrackPoint* p2){
//		//sort for z-coordinates of bottom wire end - assumes no major differences in rotation around x axis for one layer
//		return p1->getRawMeasurement()->getRawHitCoords()[2] < p2->getRawMeasurement()->getRawHitCoords()[2];
//	});


	TVector3 PCA_track(0,0,0);
	TVector3 PCA_wire(0,0,0);
	TVector3 closest_approach(0,0,0);
	int detID,driftside;
	double track_distance, rt, residual;

	//#pragma omp parallel for
	for(size_t i = 0; i < n_points; ++i)
	{
		TVector3 PCA_track_last = PCA_track;
		pair<int,double> point = points[i];
		//get coords of upper and lower end of hit tube
		TVector3 vbot;
		TVector3 vtop;
		m_survey.TubeEndPointsSurvey(point.first, vtop, vbot);
//		#pragma omp critical
//		{
//			MufluxSpectrometer::TubeEndPoints(point.first, vbot, vtop);
//		}
		if (pede_corrections)
		{
			vector<int> labels_for_tube = calc_labels(MODULE, point.first);
			double correction_x = (*pede_corrections)[labels_for_tube[0]];
//			double correction_z = (*pede_corrections)[labels_for_tube[2]];
			double correction_z = 0;

			vbot[0] = vbot[0] + correction_x;
			vtop[0] = vtop[0] + correction_x;
			vbot[2] = vbot[2] + correction_z;
			vtop[2] = vtop[2] + correction_z;
			double rotation_gamma = (*pede_corrections)[labels_for_tube[5]];
			//apply rotation
			TRotation rot;
			rot.RotateZ(rotation_gamma);
			string module_name = m_tube_id_to_module[point.first];
			TVector3 mod_center = m_nominal_module_centerpos[module_name];
			TVector3 new_top = mod_center + (rot * (vtop - mod_center));
			TVector3 new_bot = mod_center + (rot * (vbot - mod_center));
			vtop = new_top;
			vbot = new_bot;

		}
		double measurement = point.second; //rt distance [cm]

		//if(measurement < 0.5)
		//{
		//	PCA_track = PCA_track_last;
		//	continue;
		//}


		closest_approach = calc_shortest_distance(vtop,vbot,linear_model[0],linear_model[1],&PCA_wire,&PCA_track);

		//reject hit if seed track is too far off the wire
		if(closest_approach.Mag() > 2.00)
		{
			continue;
		}

		TMatrixD* jacobian;
		if (i != 0)
		{
			jacobian = calc_jacobian(PCA_track_last, PCA_track);
		}
		else
		{
			jacobian = new TMatrixD(5,5);
			jacobian->UnitMatrix();
		}
		result.push_back(gbl::GblPoint(*jacobian));
		add_measurement_info(result.back(),closest_approach, measurement, sigma_spatial);

		//calculate labels and global derivatives for hit
		vector<int> label = calc_labels(MODULE,point.first);
		TVector3 wire_bot_to_top = vtop - vbot;

		TVector3 measurement_prediction, alignment_origin;
		string module_descriptor;

		switch (mode)
		{
		case MODULE:
			module_descriptor = m_tube_id_to_module[point.first];
			alignment_origin = calc_module_centerpos(make_pair(module_descriptor, m_modules[module_descriptor]));
			measurement_prediction = PCA_track - alignment_origin;
			break;
		case SINGLE_TUBE:
			alignment_origin = vbot + 0.5 * (vtop - vbot);
			measurement_prediction = PCA_track - alignment_origin;
			break;
		}
		TMatrixD* globals = calc_global_parameters(measurement_prediction,closest_approach,linear_model,wire_bot_to_top);
		result.back().addGlobals(label, *globals);
		delete globals;
		delete jacobian;

		#pragma omp critical
		{
			if (tree)
			{
				tree->SetBranchAddress("detectorID", &detID);
				tree->SetBranchAddress("driftside", &driftside);
				tree->SetBranchAddress("trackDistance", &track_distance);
				tree->SetBranchAddress("rt", &rt);
				tree->SetBranchAddress("residual", &residual);
				tree->SetBranchAddress("wire_pca_x", &PCA_wire[0]);
				tree->SetBranchAddress("wire_pca_y", &PCA_wire[1]);
				tree->SetBranchAddress("wire_pca_z", &PCA_wire[2]);
				tree->SetBranchAddress("meas_x", &closest_approach[0]);
				tree->SetBranchAddress("meas_y", &closest_approach[1]);
				tree->SetBranchAddress("meas_z", &closest_approach[2]);
				detID = point.first;
				driftside = closest_approach[0] < 0 ? -1 : 1;
				track_distance = closest_approach.Mag();
				rt = measurement;
				residual = measurement - closest_approach.Mag();
				tree->Fill();
			}
		}
	}

	return result;
}

/**
 *
 */
void MillepedeCaller::add_measurement_info(gbl::GblPoint& point, const TVector3& closest_approach, const double measurement, const double sigma_spatial) const
{
	//define projection of measurement system to the fit system
	TMatrixD fit_system_base_vectors(2,3);
	fit_system_base_vectors.Zero();
	fit_system_base_vectors[0][0] = 1.0; 	//first row vector for x direction
	fit_system_base_vectors[1][1] = 1.0; 	//second row vector for y direction

	TRotation rotation_global_to_measurement = calc_rotation_of_vector(closest_approach);
	rotation_global_to_measurement.Invert();
	TMatrixD rot_mat = rot_to_matrix(rotation_global_to_measurement);
	TMatrixD projection_matrix = calc_projection_matrix(fit_system_base_vectors,rot_mat);
	TVectorD rotated_residual(2);
	rotated_residual[0] = measurement - closest_approach.Mag();
	rotated_residual[1] = 0;

	TVectorD precision(rotated_residual);
	precision[0] = 1.0 / TMath::Power(sigma_spatial,2);
	point.addMeasurement(projection_matrix,rotated_residual,precision);
}

/**
 * Create and initialize a ROOT TTree that contains a number of variables that can be analyzed offline.
 *
 */
TTree* MillepedeCaller::create_output_tree()
{
	TTree* tree = new TTree("DebuggingTree","Tree with debugging info");
	int det_id, driftside;
	double track_distance, rt, residual, wire_pca_x, wire_pca_y, wire_pca_z, meas_x, meas_y, meas_z;
	tree->Branch("detectorID",&det_id,"detectorID/I");
	tree->Branch("driftside",&driftside,"driftside/I");
	tree->Branch("trackDistance",&track_distance,"trackDistance/D");
	tree->Branch("rt",&rt,"rt/D");
	tree->Branch("residual",&residual,"residual/D");
	tree->Branch("wire_pca_x",&wire_pca_x,"wire_pca_x/D");
	tree->Branch("wire_pca_y",&wire_pca_y,"wire_pca_y/D");
	tree->Branch("wire_pca_z",&wire_pca_z,"wire_pca_z/D");
	tree->Branch("meas_x",&meas_x,"meas_x/D");
	tree->Branch("meas_y",&meas_y,"meas_y/D");
	tree->Branch("meas_z",&meas_z,"meas_z/D");

	return tree;
}


//TODO implement
vector<int> MillepedeCaller::calc_labels(const alignment_mode mode, const int channel_id) const
{
	switch(mode)
	{
	case MODULE:
		return labels_case_module(channel_id);
		break;
	case SINGLE_TUBE:
		return {0,0,0,0,0,0};
		break;
	case LAYER:
		return {0,0,0,0,0,0};
		break;
	default:
		return {0,0,0,0,0,0};
		break;
	}
}

vector<int> MillepedeCaller::labels_case_module(const int channel_id) const
{
	int station = -1;
	int layer = -1;
	int view = -1;
	int tube = -1;
	int pnb = -1;

	MufluxSpectrometer::TubeDecode(channel_id, station, view, pnb, layer, tube);
	vector<int> labels(6);
	int base_label = 1000 * station;
	if(station > 2)
	{
		//TODO make modules close to beam axis first in if checks
		if(tube >= 37 && tube <= 48)
		{
			base_label += 10;
		}
		else if(tube >= 25 && tube <= 36)
		{
			base_label += 20;
		}
		else if(tube >= 13 && tube <= 24)
		{
			base_label += 30;
		}
		//ADCB scheme
		//TODO change that
	}
	else
	{
		base_label += 100 * view;
	}

	for(uint8_t i = 0; i < labels.size(); ++i)
	{
		labels[i] = base_label + i + 1;
	}

	return labels;
}

/**
 * Calculate the global parameters for pede alignment. This is a matrix with dimensions 3x6 for a rigid body alignment.
 * A measurement \f$m\f$ (being a three dimensional vector, e.g x,y,z), the distorted measurement \f$\tilde{m}\f$ depends on the six\
 * (global - thus affecting ALL measurements) parameters \f$ g = {\Delta x, \Delta y, \Delta z, \alpha, \beta, \gamma}\f$.
 * Then it can be expressed as:
 * \f[
 * \tilde{m} = m + \begin{pmatrix}
 * \Delta x \\ \Delta y \\ \Delta z
 * \end{pmatrix} + \alpha \begin{pmatrix}
 *	0 \\ -z_p \\ y_p
 * \end{pmatrix} + \beta \begin{pmatrix}
 *	z_p \\ 0 \\ -x_p
 * \end{pmatrix} + \gamma \begin{pmatrix}
 * -y_p \\ x_p \\ 0
 * \end{pmatrix}
 * \f]
 *
 * @brief Calculate the global parameters for alignment
 *
 * @author Stefan Bieschke
 * @date Feb. 6, 2019
 * @version 1.1
 *
 * @param measurement_prediction predicted vector from the alignment system origin to the closest approach on the track for this hit in global coordinates
 * @param closest_approach vector of closest approach from wire to track in global reference frame
 * @param linear_model Linear track model with on-track coordinates and direction in global reference frame
 * @param Wire axis vector in global reference frame showing from wire bottom to wire top position
 *
 * @result Matrix (3x6) containing the derivatives of the measurement w.r.t all parameters
 */
TMatrixD* MillepedeCaller::calc_global_parameters(const TVector3& measurement_prediction, const TVector3& closest_approach, const vector<TVector3>& linear_model, const TVector3& wire_bot_to_top) const
{
	TRotation global_to_alignment;
	global_to_alignment.SetYAxis(wire_bot_to_top);
	global_to_alignment.Invert();
	TMatrixD mat_global_to_alignment = rot_to_matrix(global_to_alignment); //debugging
	TVector3 meas_prediction_in_alignment_system = global_to_alignment * measurement_prediction;
	TVector3 track_dir_in_alignmentsys = global_to_alignment * linear_model[1];
	TRotation global_to_measurement;
	global_to_measurement.SetXAxis(closest_approach);
	global_to_measurement.Invert();
	TMatrixD mat_global_to_measurement = rot_to_matrix(global_to_measurement); //debugging

	TVector3 meas_prediction_in_meas_system = global_to_measurement * measurement_prediction;
	TRotation alignment_to_measurement = global_to_measurement * global_to_alignment.Inverse();
	alignment_to_measurement.Invert();
	TMatrixD mat_alignment_to_measurement = rot_to_matrix(alignment_to_measurement);

	TMatrixD dmdg(3,6);
	TMatrixD* result = new TMatrixD(3,6);
	TMatrixD drdm(3,3);
	drdm.UnitMatrix();
	TVector3 nominal_measurementplane_normal(0,0,1);
	double scalar_prod = track_dir_in_alignmentsys.Dot(nominal_measurementplane_normal);

	for(short i = 0; i < drdm.GetNrows(); ++i)
	{
		for(short j = 0; j < drdm.GetNcols(); ++j)
		{
			drdm[i][j] -= track_dir_in_alignmentsys[i] * nominal_measurementplane_normal[j] / scalar_prod;
		}
	}

	dmdg.Zero();
	result->Zero();

	dmdg[0][0] = dmdg[1][1] = dmdg[2][2] = 1;
	dmdg[0][4] = meas_prediction_in_alignment_system[2];
	dmdg[0][5] = - meas_prediction_in_alignment_system[1];
	dmdg[1][3] = - meas_prediction_in_alignment_system[2];
	dmdg[1][5] = meas_prediction_in_alignment_system[0];
	dmdg[2][3] = meas_prediction_in_alignment_system[1];
	dmdg[2][4] = -meas_prediction_in_alignment_system[0];

	TMatrixD global_derivatives_in_alignment_system(3,6);
	global_derivatives_in_alignment_system.Mult(drdm, dmdg);
	result->Mult(mat_alignment_to_measurement,global_derivatives_in_alignment_system);

	return result;
}


//TODO document
/**
 *
 */
gbl::GblTrajectory MillepedeCaller::perform_GBL_refit(const genfit::Track& track, double sigma_spatial, map<int,double>* pede_corrections, const char* spillname)
{
	GBL_seed_track seed(track);
	try
	{
		return perform_GBL_refit(seed, sigma_spatial, pede_corrections, spillname);
	}
	catch(const exception& e)
	{
		throw e;
	}

}

gbl::GblTrajectory MillepedeCaller::perform_GBL_refit(const GBL_seed_track& track, double sigma_spatial,map<int,double>* pede_corrections, const char* spillname)
{

	vector <gbl::GblPoint> points = list_hits(&track, MODULE, sigma_spatial, pede_corrections, m_output_tree);
	gbl::GblTrajectory traj(points,false); //param false for B=0

	traj.milleOut(*m_gbl_mille_binary);
	//check track validity
	if(!traj.isValid())
	{
		cout << "Error, GBL trajectory is invalid." << endl;
		cerr << "Error, GBL trajectory is invalid." << endl;
		throw runtime_error("GBL track is invalid");
	}

	int rc, ndf;
	double chi2, lostWeight;
//	cout << "Seed slope (dx,dy): " << track.get_direction()[0]/track.get_direction()[2] << ", " << track.get_direction()[1]/track.get_direction()[2] << endl;

	cout << "------------performing refit--------------" << endl;

	rc = traj.fit(chi2,ndf,lostWeight);
	cout << "Refit chi2: " << chi2 << " Ndf: " << ndf << endl;
	cout << "Prob: " << TMath::Prob(chi2,ndf) << endl;
	print_fitted_residuals(traj);
//	print_fitted_track(traj);

	return traj;
}

//TODO document
double MillepedeCaller::MC_GBL_refit(unsigned int n_tracks, double smearing_sigma, unsigned int min_hits, map<int,double>* pede_corrections)
{

	if(pede_corrections)
	{
		cout << "Fitting with corrections:" << endl;

		for(map<int,double>::iterator it = pede_corrections->begin(); it != pede_corrections->end(); ++it)
		{
			cout << it->first << " " << it->second << endl;
		}
	}
	vector<int> shifted_det_ids = { };
//	vector<string> shifted_modules = { "T3aX", "T3bX", "T3cX", "T3dX", "T4aX", "T4bX", "T4cX", "T4dX" };
	vector<string> shifted_modules = {};
	for (string mod : shifted_modules)
	{
		for (int id : m_modules[mod])
		{
			shifted_det_ids.push_back(id);
		}
	}
	double chi2, lostweight;
	int ndf;
	vector<vector<TVector3>> tracks(n_tracks);

	for(unsigned int i = 0; i < n_tracks; ++i)
	{
		//case for boosted tracks
//		tracks[i] = MC_gen_track_boosted();
		tracks[i] = MC_gen_track();
	}


	ofstream file("MC_slopes.txt");
//
//	vector<vector<TVector3>> sampled_tracks = resample_tracks(tracks);
//	tracks = sampled_tracks;


	unsigned int fitted = 0;
	#pragma omp parallel for
	for(size_t i = 0; i < tracks.size(); ++i)
	{
		auto track = tracks[i];
		vector<pair<int,double>> hits = MC_gen_clean_hits(track[0], track[1],&shifted_det_ids);
		if(hits.size() < min_hits)
		{
			continue;
		}
		smear_hits(hits,350e-4);

		GBL_seed_track seed(track,hits);
		file << seed.get_direction()[0]/seed.get_direction()[2] << "\t" << seed.get_direction()[1]/seed.get_direction()[2] << endl;
		perform_GBL_refit(seed, 350e-4, pede_corrections);

		#pragma omp atomic
		++fitted;
	}
	file.close();
//	tree->Write();
//	debuggingfile.Close();

	return 0.0;
}


void MillepedeCaller::write_resolution_function(const char* filename,
		const genfit::Track& track,
		std::vector<MufluxSpectrometerHit>* raw_hits) const
{
	/*
	 * File output of distance(track-wire) vs. residuals
	 */
	ofstream resolutionfunction("resolution.ascii", ios::app);
	vector<TVector3> linear_model = linear_model_wo_scatter(track);
	vector<MufluxSpectrometerHit*>* fitted = nullptr;
	vector<genfit::TrackPoint*> points = track.getPointsWithMeasurement();
	size_t n_points = points.size();
	if (raw_hits)
	{
		fitted = new vector<MufluxSpectrometerHit*>(0);
		for (unsigned int i = 0; i < n_points; ++i)
		{
			genfit::TrackPoint* point = points[i];
			int id = point->getRawMeasurement()->getDetId();
			for (unsigned int j = 0; j < raw_hits->size(); ++j)
			{
				MufluxSpectrometerHit raw = raw_hits->at(j);
				if (raw.GetDetectorID() == id)
				{
					fitted->push_back((MufluxSpectrometerHit*) &(raw_hits->at(j)));
				}
			}
		}
	}


	for(size_t i = 0; i < n_points; ++i)
	{
		genfit::TrackPoint* point = points[i];
		//get coords of upper and lower end of hit tube
		genfit::AbsMeasurement* raw_measurement = point->getRawMeasurement();
		int det_id = raw_measurement->getDetId();
		TVectorD raw = raw_measurement->getRawHitCoords();
		TVector3 vbot(raw[0],raw[1],raw[2]);
		TVector3 vtop(raw[3],raw[4],raw[5]);
		double measurement = raw[6]; //rt distance [cm]

		TVector3 closest_approach = calc_shortest_distance(vtop,vbot,linear_model[0],linear_model[1]);

		//write ascii output for testing
		resolutionfunction << closest_approach.Mag() << "\t" << closest_approach.Mag() - measurement;
		if(fitted)
		{
			resolutionfunction << "\t" << fitted->at(i)->GetTimeOverThreshold();
		}
		resolutionfunction << endl;

	}
	resolutionfunction.close();
	if(fitted) delete fitted;
}

//Reimplementation of python function
/**
 * Calculates a vector the represents the shortest distance from the sense wire to the track. This vector is perpendicular both
 * to the track as well as the sense wire.
 *
 * @brief Calculates shortest distance from sense wire to track
 *
 * @author Stefan Bieschke
 * @date Oct. 01, 2019
 * @version 1.1
 *
 * @param wire_top Top position of the sense wire as TVector3 with x,y,z components
 * @param wire_bot Bottom position of the sense wire as TVector3 with x,y,z components
 * @param track_pos Some position on the track. Could be anything but must be on the (straight) track or track segment
 * @param track_mom Momentum vector of the (straight) track or track segment
 * @param PCA_on_wire Coordinates of point of closest approach (PCA) on the wire (defaults to nullptr)
 * @param PCA_on_track Coordinates of point of closest approach (PCA) on the track (defaults to nullptr)
 *
 * @return TVector3 of shortest distance pointing from the wire to the track
 */
TVector3 MillepedeCaller::calc_shortest_distance(const TVector3& wire_top,
		const TVector3& wire_bot, const TVector3& track_pos,
		const TVector3& track_mom, TVector3* PCA_on_wire,
		TVector3* PCA_on_track) const
{
	TVector3 wire_dir = wire_top - wire_bot;

	//construct a helper plane that contains one of the straights and is parallel to the other one
	TVector3 plane_pos = track_pos - wire_bot;
	TVector3 plane_dir_1(track_mom);
	TVector3 plane_dir_2(-1 * wire_dir);

	//Construct components of equation system M * x = c where M is the coefficient matrix, x the solution and c the const_vector below
	TVectorD const_vector(2);
	TMatrixD coeff_matrix(2,2);

	const_vector[0] = -(plane_pos.Dot(track_mom));
	const_vector[1] = -(plane_pos.Dot(wire_dir));

	coeff_matrix[0][0] = plane_dir_1.Dot(track_mom);
	coeff_matrix[0][1] = plane_dir_2.Dot(track_mom);
	coeff_matrix[1][0] = plane_dir_1.Dot(wire_dir);
	coeff_matrix[1][1] = plane_dir_2.Dot(wire_dir);

	TDecompLU solvable_matrix(coeff_matrix);
	TVectorD result(const_vector);
	int rc = solvable_matrix.Solve(result);

	TVector3 PCA_track(track_pos + result[0] * track_mom);
	TVector3 PCA_wire(wire_bot + result[1] * wire_dir);

	//if TVector3 pointers were passed as parameter the actual locations of closest approach are stored there
	if(PCA_on_wire)
	{
		*(PCA_on_wire) = PCA_wire;
	}
	if(PCA_on_track)
	{
		*(PCA_on_track) = PCA_track;
	}

	return TVector3(PCA_track - PCA_wire);
}

/**
 * Calculate the rotation matrix in a way such that a given vector v is the x-axis of the rotated coordinate frame.
 *
 * @brief Rotation of a given vector in lab frame so that it is new x axis
 *
 * @author Stefan Bieschke
 * @date Aug. 09, 2019
 * @version 1.0
 *
 * @param v TVector3 that is meant to be the new x axis of the rotated coordinate frame
 *
 * @return TRotation containing the rotation matrix
 */
TRotation MillepedeCaller::calc_rotation_of_vector(const TVector3& v) const
{
	TRotation rot;
	rot.SetXAxis(v);

	return rot;
}

/**
 * Convert a TRotation to a its 3x3 rotation matrix given as TMatrixD
 *
 * @brief Convert a TRotation to a (3x3) TMatrixD
 *
 * @author Stefan Bieschke
 * @date Sep. 09, 2019
 * @version 1.0
 *
 * @param rot TRotation object for that the rotation matrix is needed
 *
 * @return TMatrixD object with dimensions 3x3
 */
TMatrixD MillepedeCaller::rot_to_matrix(const TRotation& rot) const
{
	TMatrixD result(3,3);
	for(uint8_t i = 0; i < 3; ++i)
	{
		for(uint8_t j = 0; j < 3; ++j)
		{
			result[i][j] = rot[i][j];
		}
	}
	return result;
}

/**
 * Calculate a position and a direction vector for a linear model that models a given track from genfit. While the track
 * from genfit might take scattering in detector material into account, this linear model doesn't consider scatterers.
 * The way this works is to calculate and return a straight track between the first and the last hit of the passed track.
 *
 * @brief Linear model of the passed track
 *
 * @author Stefan Bieschke
 * @date Sep. 09, 2019
 * @version 1.0
 *
 * @param track Track that is meant to be modeled
 *
 * @return std::vector of length two, whos first entry is the position vector, second one being the direction vector
 */
vector<TVector3> MillepedeCaller::linear_model_wo_scatter(const genfit::Track& track) const
{
	vector<TVector3> result(2);

	//get first and last fitted states
	size_t n_hits = track.getNumPointsWithMeasurement();
	genfit::StateOnPlane first_hit = track.getFittedState(0);
	genfit::StateOnPlane last_hit = track.getFittedState(n_hits - 1);

	//position is fitted position on first hit
	TVector3 pos = first_hit.getPos();

	//direction is difference between positions at first and last hits
	TVector3 dir = last_hit.getPos() - first_hit.getPos();
	result[0] = pos;
	result[1] = dir;

	return result;
}

/**
 * Calculate the projection matrix (dimension 2x2) from the global reference frame (x,y,z) to the measurement system (u,v,w) for an
 * individual hit.
 *
 * @brief Calculate projection matrix global to measumrent system
 *
 * @author Stefan Bieschke
 * @date Oct. 07, 2019
 * @version 1.0
 *
 * @param fit_system_base_vectors Matrix with dimensions 2x3 with the base vectors of the local fit system expressed in global system. E.g [(1., 0., 0.),(0.,1.,0.)] for x,y and track in general z direction.
 * @param rotation_global_to_measurement Rotation matrix rotating the global frame to the measurement frame
 *
 * @return TMatrixD (dimension 2x2) containing the projection of the fit system (x,y) on the measurement system
 */
TMatrixD MillepedeCaller::calc_projection_matrix(
		const TMatrixD& fit_system_base_vectors,
		const TMatrixD& rotation_global_to_measurement) const
{
	TMatrixD result(2,2); //projection matrix has dimensions 2x2
	TMatrixD measurement_to_global(rotation_global_to_measurement); //copy rotational matrix
	measurement_to_global.Invert(); //invert matrix in place
	measurement_to_global.ResizeTo(3,2); //TODO check if this is correct, want to skip column normal to measurement direction
	result.Mult(fit_system_base_vectors,measurement_to_global);
	result.Invert();
	return result;
}

/**
 * Calculate the nominal, geometric center position of a drift tube module in global coordinates. The nominal coordinates are taken
 * from the c++ class MufluxSpectrometer.
 *
 * @brief Calculate module center position in global coordinates
 *
 * @author Stefan Bieschke
 * @date Feb. 12, 2020
 * @version 1.0
 *
 * @param module_name_id_list_pair pair of string with module descriptor (e.g "T1X") in first entry and list of ids in second
 */
TVector3 MillepedeCaller::calc_module_centerpos(const pair<string,vector<int>>& module_name_id_list_pair) const
{
	TVector3 center_pos;
	TVector3 wire_center;
	TVector3 wire_top, wire_bot;
	int n_tubes = 0;
	for(int id : module_name_id_list_pair.second)
	{
		m_survey.TubeEndPointsSurvey(id, wire_top, wire_bot);
//		#pragma omp critical
//		{
//			MufluxSpectrometer::TubeEndPoints(id, wire_bot, wire_top);
//		}
		//calculate centerpos of single wire
		wire_center = wire_bot + 0.5 * (wire_top - wire_bot);
		center_pos += wire_center;
		++n_tubes;
	}
	center_pos = (1.0 / n_tubes) * center_pos;

	return center_pos;
}


void MillepedeCaller::print_model_parameters(const vector<TVector3>& model) const
{
	double slope_x = model[1].X() / model[1].Z();
	double slope_y = model[1].Y() / model[1].Z();

	cout << "Printing linear track model parameters" << endl;
	cout << "Initial fit position:" << endl;
	model[0].Print();
	cout << "Track direction:" << endl;
	model[1].Print();
	cout << "Model parameters: x0, y0, slope x, slope y" << endl;
	cout << "(" << model[0].X() << ", " << model[0].Y() << ", " << slope_x  << ", " << slope_y <<")" << endl;
}

/**
 * Checks, if the points on a genfit seed track are ordered by arc length. The arc length
 * is the distance from an early point on the track, e.g the vertex or the very first hit.
 * This doesn't matter if the arc length itself is not used but the GblPoints must only
 * be ordered correctly.
 *
 * @brief Checks if points on seed track are ordered by arc length
 *
 * @author Stefan Bieschke
 * @date Nov. 6, 2019
 * @version 1.0
 *
 * @param track genfit seed track
 * @return true if points are correctly ordered, false else
 *
 * @note This is mainly for debugging and maintenance purposes, not recommended to be called in productive code
 */
bool MillepedeCaller::check_ordered_by_arclen(const genfit::Track& track) const
{
	size_t n_points = track.getNumPointsWithMeasurement();
	double z = 0.0;

	for(size_t i = 0; i < n_points; ++i)
	{
		double z_next = track.getFittedState(i).getPos().Z();
		if(z_next > z)
		{
			z = z_next;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void MillepedeCaller::print_seed_hits(const genfit::Track& track) const
{
	size_t n_hits = track.getNumPointsWithMeasurement();
	const vector< genfit::TrackPoint* > points = track.getPoints();

	for(size_t i = 0; i < n_hits; ++i)
	{
		int det_id = points[i]->getRawMeasurement()->getDetId();
		cout << "Hit: " << i << "\t" << "ID: " << det_id << endl;
	}
}

/**
 * Prints the residuals of the fitted trajectory to all the measurements, the GblTrajectory contains in the form of
 * GblPoint objects. This is outputted as text.
 *
 * @brief Text output of residuals for each hit of fitted GblTrajectory
 *
 * @author Stefan Bieschke
 * @date Jan. 24, 2020
 * @version 1.0
 *
 * @parameter trajectory GblTrajectory that is already fitted
 */
void MillepedeCaller::print_fitted_residuals(gbl::GblTrajectory& trajectory) const
{
	//print residuals
	TVectorD residuals(1);
	TVectorD meas_errors(1);
	TVectorD res_errors(1);
	TVectorD down_weights(1);
	unsigned int numRes;
	for (unsigned int j = 1; j <= trajectory.getNumPoints(); ++j)
	{
		trajectory.getMeasResults(j, numRes, residuals, meas_errors, res_errors,down_weights);
		cout << "Hit: " << j << " Residual: " << residuals[0] << endl;
	}
}

void MillepedeCaller::print_fitted_track(gbl::GblTrajectory& trajectory) const
{
	TVectorT<double> parameters(5);
	TMatrixTSym<double> covariance(5, 5);
	for (unsigned int i = 1; i <= trajectory.getNumPoints(); ++i)
	{
		trajectory.getResults(i, parameters, covariance);
		cout << "Hit: " << i << endl;
		for (unsigned int j = 0; j < parameters.GetNrows(); ++j)
		{
			cout << "Parameter: " << j << " value: " << parameters[j] << endl;
		}
	}
}


/**
 * Generates a very simple toy MC track. There is no physics involved, this is just meant to test the GBL refit. The
 * MC tracks return a vector<TVector3> containing two TVector3 objects. The first being a position in front of T1, the second
 * one a direction, the track propagates from there. These are only straight lines, no scattering or any physics involved.
 *
 * @brief Generate a very simple toy MC track
 *
 * @author Stefan Bieschke
 * @date Nov. 22, 2019
 * @version 1.0
 *
 * @return std::vector<TVector3> two element vector with first element position (e.g vertex) and second element direction
 */
vector<TVector3> MillepedeCaller::MC_gen_track()
{
	/*
	 * Track locations uniformly distributed in front of first plane and in front of first plane of T4.
	 *
	 * area for x and y is calculated as follows:
	 * allowed x area is centerpos +- 21 cm for T1, (centerpos T4d - 21cm) to (centerpos T4a + 21cm) for T4
	 * allowed y area is centerpos +- 21 cm for T1, centerpos +- 75 cm for T4
	 *
	 * Positions below calculated with: https://github.com/StBies/FairShip/blob/a7ead53a260a1b796371eb09ec406421c027ad18/charmdet/drifttubeMonitoring.py#L955
     * Module: T1X Centerpos = 1.8225      -1.995  24.99
     * Module: T1U Centerpos = 1.33890796894       0.782207722399  56.20825
     * Module: T2V Centerpos = 1.71329088596       0.663508489501  93.68275
     * Module: T2X Centerpos = 1.03        -2.0875 125.082
     * Module: T3aX Centerpos = 76.9353333333       10.512  583.19
	 * Module: T3bX Centerpos = 25.987      10.289  585.065
     * Module: T3cX Centerpos = -24.539     10.28075        585.1175
     * Module: T3dX Centerpos = -75.0575    10.23975        585.315
     * Module: T4aX Centerpos = 76.74       10.163  748.2675
     * Module: T4bX Centerpos = 26.3425     9.939   748.1975
     * Module: T4cX Centerpos = -24.0075    9.8685  748.27
	 * Module: T4dX Centerpos = -74.48      9.92025 748.4325
	 */
	uniform_real_distribution<double> uniform(0.0, 1.0);
	double offset_x_beginning = m_nominal_module_centerpos["T1X"][0] - 21.0 + 42.0 * uniform(m_mersenne_twister);
	double offset_y_beginning = m_nominal_module_centerpos["T1X"][1] -21.0 + 42.0 * uniform(m_mersenne_twister);
	double downstream_station_width = m_nominal_module_centerpos["T4aX"][0] - m_nominal_module_centerpos["T4dX"][0] + 42.0;
	double offset_x_end = m_nominal_module_centerpos["T4dX"][0] - 21.0 + downstream_station_width * uniform(m_mersenne_twister);
	double offset_y_end = m_nominal_module_centerpos["T4dX"][1] - 75.0 + 150.0 * uniform(m_mersenne_twister);

	double z_beginning = 17.0;
	double z_end = 739.0;

	TVector3 beginning(offset_x_beginning, offset_y_beginning, z_beginning);
	TVector3 end(offset_x_end, offset_y_end, z_end);
	TVector3 direction = end - beginning;
	vector<TVector3> result = {beginning, direction};

	cout << "Beg: (" << beginning[0] << "," << beginning[1] << "," << beginning[2] << ")"
				<< "End : (" << end[0] << "," << end[1] << "," << end[2] << ")" << endl;

	return result;
}

vector<TVector3> MillepedeCaller::MC_gen_track_boosted()
{
	uniform_real_distribution<double> uniform(0.0,1.0);
	normal_distribution<double> gaussian(0,3.0);
	//generate point somewhere near target
	double z_beg = -400 + 80 * uniform(m_mersenne_twister);
	double r_beg = 5 * uniform(m_mersenne_twister);
	double rotation_beg = 2 * TMath::Pi() * uniform(m_mersenne_twister);
	double x_beg = r_beg * TMath::Cos(rotation_beg);
	double y_beg = r_beg * TMath::Sin(rotation_beg);
	TVector3 beginning(x_beg, y_beg, z_beg);

	//generate gaussian distribution of points close to center of T1 with width of 12.5cm in x and y
	double r_end = gaussian(m_mersenne_twister);
	double rotation_end = 2 * TMath::Pi() * uniform(m_mersenne_twister);
	double x_end = r_end * TMath::Cos(rotation_end);
	double y_end = r_end * TMath::Sin(rotation_end);
	TVector3 end(x_end, y_end, 17);

	vector<TVector3> result = {beginning, end};
//	cout << "Beg: (" << beginning[0] << "," << beginning[1] << "," << beginning[2] << ")"
//					<< "End : (" << end[0] << "," << end[1] << "," << end[2] << ")" << endl;
	return result;
}


vector<vector<TVector3>> MillepedeCaller::resample_tracks(const vector<vector<TVector3>>& tracks)
{
	vector<vector<TVector3>> result = {};
	result.reserve(tracks.size());

	//find min and max slope to define borders of probability histogram
	double min_slope = 100;
	double max_slope = -100;
	for(size_t i = 0; i < tracks.size(); ++i)
	{
		double slope_x = tracks[i][1][0] / tracks[i][1][2];
		min_slope = slope_x < min_slope ? slope_x : min_slope;
		max_slope = slope_x > max_slope ? slope_x : max_slope;
	}

	TH1D slopes("slopes","slope distribution",200, 1.1 * min_slope, 1.1 * max_slope);
	for(auto track: tracks)
	{
		double slope = track[1][0] / track[1][2];
		slopes.Fill(slope);
	}
	TH1D sampling_probability(slopes);
	sampling_probability.SetNameTitle("prob", "sampling probability");
	unsigned int mean_bin_content = (int)(0.01 * slopes[slopes.GetMaximumBin()]); //10 percent of maximum bin content of unsampled slope dist
	cout << "Resampling with " << mean_bin_content << " tracks per bin in average" << endl;
	for(size_t i = 0; i < slopes.GetNbinsX(); ++i)
	{
		sampling_probability[i] = mean_bin_content / slopes[i];
	}
	uniform_real_distribution<double> uniform(0,1);
	for(vector<TVector3> track: tracks)
	{
		double p = uniform(m_mersenne_twister);
		double slope = track[1][0] / track[1][2];
		int bin = sampling_probability.FindBin(slope);
		if(p < sampling_probability[bin])
		{
			result.push_back(track);
		}
	}
	result.shrink_to_fit();

	return result;
}

/**
 * Generate unsmeared hits for a given MC track. The track is passed with its startpoint and direction to this method.
 * When generating hits the closest distance to the wire is calculated for every wire in the detector to this track. If the
 * distance is smaller 18.15mm, a hit is written. The hits are stored in a vector<pair<int,double>>. The vector is as long as
 * the number of hits that were found. Each element is a pair of int and double, the int being the hit detectorID, the double
 * being the unsmeared hit distance (drift distance). The hits are sorted by arc length (which is the distance from the very
 * first hit).
 *
 * @brief Generate MC hits for a given track
 *
 * @author Stefan Bieschke
 * @date Nov. 22, 2019
 * @version 1.0
 *
 * @param start Starting point (like vertex) of MC track
 * @param direction Direction of the MC track
 *
 * @return std::vector<std::pair<int,double>> list of pairs of int and double, the first one being the detectorID, the second one the rt distance
 */
vector<pair<int,double>> MillepedeCaller::MC_gen_clean_hits(const TVector3& start, const TVector3& direction, const vector<int>* shifted_det_ids)
{
//	cout << "Beginning unsmeared hit generation" << endl;
	vector<pair<int,double>> result(0);
	TVector3 wire_end_top;
	TVector3 wire_end_bottom;
	TVector3 wire_to_track;

	//check distance to every tube
	for(auto entry : m_tube_id_to_module)
	{
		int id = entry.first;
//		m_survey.TubeEndPointsSurvey(id, wire_end_top, wire_end_bottom);
		#pragma omp critical
		{
			MufluxSpectrometer::TubeEndPoints(id, wire_end_bottom, wire_end_top);
		}
		if(shifted_det_ids)
		{
			TVector3 translation(0.03536, 0, -0.03536); //realistic
			TRotation rot;
			rot.RotateZ(6.25e-4); //0.036 deg, means 500um offset at wire end
			bool id_shifted = shifted_det_ids->end() != find(shifted_det_ids->begin(),shifted_det_ids->end(),id);
			if(id_shifted)
			{
				vector<TVector3> top_bot_new = rotate_tube_in_module(id, rot);
				wire_end_top = top_bot_new[0];
				wire_end_bottom = top_bot_new[1];
				wire_end_bottom = wire_end_bottom + translation;
				wire_end_top = wire_end_top + translation;
//				cout << "Top: (" << wire_end_top[0] << ", " << wire_end_top[1] << ", " << wire_end_top[2] << ")" << endl;
			}
		}
		wire_to_track = calc_shortest_distance(wire_end_top, wire_end_bottom, start, direction, nullptr, nullptr);

		if(wire_to_track.Mag() < 1.815)
		{
			result.push_back(pair<int,double>(id,wire_to_track.Mag()));
		}
	}

	//sort with lambda comparison

	sort(result.begin(), result.end(), [&](pair<int,double> element1, pair<int,double> element2){
//		m_survey.TubeEndPointsSurvey(element1.first, wire_end_top, wire_end_bottom);
		#pragma omp critical
		{
			MufluxSpectrometer::TubeEndPoints(element1.first, wire_end_bottom, wire_end_top);
		}
		double z1 = (wire_end_bottom + ((wire_end_top - wire_end_bottom)* 0.5)).Z();
//		m_survey.TubeEndPointsSurvey(element2.first, wire_end_top, wire_end_bottom);
		#pragma omp critical
		{
			MufluxSpectrometer::TubeEndPoints(element2.first, wire_end_bottom, wire_end_top);
		}
		double z2 = (wire_end_bottom + ((wire_end_top - wire_end_bottom)* 0.5)).Z();
		return z1 < z2;
		});

	return result;
}

void MillepedeCaller::smear_hits(vector<pair<int,double>>& unsmeared, const double sigma)
{
	normal_distribution<double> gaussian_smear(0,sigma);
	double smear;
	for(size_t i = 0; i < unsmeared.size(); ++i)
	{
		smear = gaussian_smear(m_mersenne_twister);
		unsmeared[i].second = TMath::Abs(unsmeared[i].second + smear);
	}
}

/**
 * Calculate the jacobi matrix for a linear model from the points of closest approach on the track for two consecutive hits.
 *
 * @brief Calculate jacobian between two consecutive hits
 *
 * @author Stefan Bieschke
 * @date November 22, 2019
 * @version 1.0
 *
 * @param PCA_1 Point of closest approach on the track for the first hit
 * @param PCA_2 Point of closest approach on the track for the second hit
 *
 * @return Pointer to a heap object of TMatrixD type with dimensions 5x5
 *
 * @warning Heap object without auto deletion
 */
TMatrixD* MillepedeCaller::calc_jacobian(const TVector3& PCA_1, const TVector3& PCA_2) const
{
	TMatrixD* jacobian = new TMatrixD(5,5);

	// 1.) init unity matrix
	jacobian->UnitMatrix();

	double dz = PCA_2.Z() - PCA_1.Z();

	//2.) enter non-zero off diagonal elements
	(*jacobian)[3][1] = dz;
	(*jacobian)[4][2] = dz;

	return jacobian;
}

/**
 * Calculate top- and bottom end positions of a drift tube's sense wire when the module of tubes that contains this particular
 * one is rotated from its nominal position
 *
 * @author Stefan Bieschke
 * @version 1.0
 *
 * @param tube_id Detector ID of the tube
 * @param rot Rotation as a ROOT TRotation object
 */
vector<TVector3> MillepedeCaller::rotate_tube_in_module(const int tube_id, const TRotation& rot)
{
	string module = m_tube_id_to_module[tube_id];
	TVector3 module_center = m_nominal_module_centerpos[module];

	TVector3 vtop, vbot;
	m_survey.TubeEndPointsSurvey(tube_id, vtop, vbot);
//	#pragma omp critical
//	{
//		MufluxSpectrometer::TubeEndPoints(tube_id, vbot, vtop);
//	}

	TVector3 new_top = module_center + rot * (vtop - module_center);
	TVector3 new_bot = module_center + rot * (vbot - module_center);

	vector<TVector3> result = {new_top, new_bot};
	return result;
}


