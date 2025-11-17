#include "sndShowerTools.h"

#include <vector>
#include <algorithm>
#include <utility>

#include "sndConfiguration.h"
#include "sndScifiPlane.h"
#include "sndUSPlane.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "TMatrixDSymEigen.h"
#include "TVectorD.h"
#include "TDecompChol.h"
#include "Math/Vector3D.h"
#include "Math/Point3D.h"

int snd::analysis_tools::GetScifiShowerStart(const std::vector<snd::analysis_tools::ScifiPlane> &scifi_planes)
{
  // Assuming planes are ordered
  for (const auto &p : scifi_planes)
  {
    if (p.HasShower())
    {
      return p.GetStation();
    }
  }
  return -1;
}

int snd::analysis_tools::GetScifiShowerEnd(const std::vector<snd::analysis_tools::ScifiPlane> &scifi_planes)
{
  // Assuming planes are ordered
  for (auto p = scifi_planes.rbegin(); p != scifi_planes.rend(); p++) {
    if (p->HasShower())
    {
      return p->GetStation();
    }
  }
  return -1;
}

int snd::analysis_tools::GetUSShowerStart(const std::vector<snd::analysis_tools::USPlane> &us_planes)
{
  // Assuming planes are ordered
  for (const auto &p : us_planes)
  {
    if (p.HasShower())
    {
      return p.GetStation();
    }
  }
  return -1;
}

int snd::analysis_tools::GetUSShowerEnd(const std::vector<snd::analysis_tools::USPlane> &us_planes)
{
  // Assuming planes are ordered
  for (auto p = us_planes.rbegin(); p != us_planes.rend(); p++) {
    if (p->HasShower())
    {
      return p->GetStation();
    }
  }
  return -1;
}

std::pair<ROOT::Math::XYZPoint, ROOT::Math::XYZVector> snd::analysis_tools::GetShowerInterceptAndDirection(const snd::Configuration &configuration, const std::vector<snd::analysis_tools::ScifiPlane> &scifi_planes, const std::vector<snd::analysis_tools::USPlane> &us_planes)
{
  std::vector<double> positions_x;
  std::vector<double> positions_y;
  std::vector<double> positions_z;
  std::vector<double> err_positions_x;
  std::vector<double> err_positions_y;
  std::vector<double> err_positions_z;

  auto collect_centroids = [&](const auto &planes) {
    for (const auto &p : planes) {
      auto centroid = p.GetCentroid();
      auto centroid_error = p.GetCentroidError();
      // Check that centroid is valid
      if (!(std::isnan(centroid.X()) || std::isnan(centroid.Y()) || std::isnan(centroid.Z()))) {
        positions_x.push_back(centroid.X());
        positions_y.push_back(centroid.Y());
        positions_z.push_back(centroid.Z());
        err_positions_x.push_back(centroid_error.X());
        err_positions_y.push_back(centroid_error.Y());
        err_positions_z.push_back(centroid_error.Z());
      }
    }
  };
  
  collect_centroids(scifi_planes);
  collect_centroids(us_planes);

  if (static_cast<int>(positions_z.size()) < configuration.centroid_min_valid_station) {
    return std::make_pair(ROOT::Math::XYZPoint(std::nan(""), std::nan(""), std::nan("")), ROOT::Math::XYZVector(std::nan(""), std::nan(""), std::nan("")));
  }

  auto weighted_linear_fit = [](const std::vector<double> &z, const std::vector<double> &val, const std::vector<double> &err)
  {
    const int n_points = z.size();
    const int n_variables = 2; // intercept + slope

    // Wrap existing memory into TVectorD
    TVectorD v_z; 
    v_z.Use(n_points, const_cast<double*>(z.data()));
    TVectorD v_y; 
    v_y.Use(n_points, const_cast<double*>(val.data()));
    TVectorD v_e; 
    v_e.Use(n_points, const_cast<double*>(err.data()));

    TMatrixD matrix(n_points, n_variables);
    TMatrixDColumn(matrix,0) = 1.0;
    TMatrixDColumn(matrix,1) = v_z;

    // Solve normal equations (weighted least squares)
    TVectorD coeffs = NormalEqn(matrix, v_y, v_e);

    return std::make_pair(coeffs[0], coeffs[1]); // (intercept, slope)
  };

  auto [intercept_x, slope_x] = weighted_linear_fit(positions_z, positions_x, err_positions_x);
  auto [intercept_y, slope_y] = weighted_linear_fit(positions_z, positions_y, err_positions_y);

  ROOT::Math::XYZPoint shower_intercept(intercept_x, intercept_y, 0.0);
  ROOT::Math::XYZVector shower_direction(slope_x, slope_y, 1.0);
  
  return std::make_pair(shower_intercept, shower_direction.Unit());
}

std::pair<std::vector<snd::analysis_tools::ScifiPlane>, std::vector<snd::analysis_tools::USPlane>> snd::analysis_tools::GetShoweringPlanes(const std::vector<snd::analysis_tools::ScifiPlane> &scifi_planes, const std::vector<snd::analysis_tools::USPlane> &us_planes)
{
  std::vector<snd::analysis_tools::ScifiPlane> sh_scifi_planes;
  std::vector<snd::analysis_tools::USPlane> sh_us_planes;

  // Filter showering Scifi planes
  std::copy_if(scifi_planes.begin(), scifi_planes.end(), std::back_inserter(sh_scifi_planes), [](const snd::analysis_tools::ScifiPlane& p) { return p.HasShower(); });
  // Filter showering US planes
  std::copy_if(us_planes.begin(), us_planes.end(), std::back_inserter(sh_us_planes), [](const snd::analysis_tools::USPlane& p) { return p.HasShower(); });

  for (auto &p: sh_scifi_planes) {
    p.FindCentroid();
  }
  for (auto &p: sh_us_planes) {
    p.FindCentroid();
  }

  return std::make_pair(sh_scifi_planes, sh_us_planes);
}

std::pair<double, double>PCA(const std::vector<double>& u, const std::vector<double>& z)
{
  // Need at least 3 measurements
  assert(u.size() == z.size() && u.size() >= 2);
  const size_t n_samples = u.size();
  const int n_variables = 2;

  // Find the mean values
  double mean_u = 0.0, mean_z = 0.0;
  for (size_t i = 0; i < n_samples; ++i)
  {
    mean_u += u[i];
    mean_z += z[i];
  }
  mean_u /= n_samples;
  mean_z /= n_samples;

  // Find the covariance components
  double cov_uu = 0.0, cov_zz = 0.0, cov_uz = 0.0;
  for (size_t i = 0; i < n_samples; ++i)
  {
    double du = u[i] - mean_u;
    double dz = z[i] - mean_z;
    cov_uu += du * du;
    cov_zz += dz * dz;
    cov_uz += du * dz;
  }

  const double denom = (n_samples > 1) ? (n_samples - 1.0) : 1.0;
  cov_uu /= denom;
  cov_zz /= denom;
  cov_uz /= denom;

  // Fill the covariance matrix
  TMatrixDSym cov_matrix(n_variables);
  cov_matrix[0][0] = cov_uu;
  cov_matrix[0][1] = cov_uz;
  cov_matrix[1][0] = cov_uz;
  cov_matrix[1][1] = cov_zz;

  // Find the eigenvalues and eigenvectors. For now we only need the former.
  // Perhaps the eigenvectors could be used in the future, so left that part in comments.
  TMatrixDSymEigen eig_decomp(cov_matrix);
  TVectorD evals = eig_decomp.GetEigenValues();
  //TMatrixD evecs = eig_decomp.GetEigenVectors();

  // Make sure the order is descending, swap if needed
  if (evals[0] < evals[1])
  {
    std::swap(evals[0], evals[1]);
    //evecs[0] = eig_decomp.GetEigenVectors()[1];
    //evecs[1] = eig_decomp.GetEigenVectors()[0];
  }

  return {evals[0], evals[1]};
}

std::pair<double, double> snd::analysis_tools::GetSciFiSpatialAnisotropy(const std::vector<ScifiPlane> &scifi_planes, bool use_all_centroids)
{
  std::vector<double> x, y, zx, zy;
  for (auto &p : scifi_planes) {
    // Add measurements per projection and in the respective coordinate vectors
    if (use_all_centroids == false) {
      for (auto &hit : p.GetHits()) {
        if (hit.is_x) {
          x.push_back(hit.x);
          zx.push_back(hit.z);
        } else {
          y.push_back(hit.y);
          zy.push_back(hit.z);
        }
      }
    } else {
      auto centroid = p.GetCentroid();
      // Check that centroid is valid
      if (!std::isnan(centroid.Z())) {
         if (!std::isnan(centroid.X())) {
            x.push_back(centroid.X());
            zx.push_back(centroid.Z());
         }
         if (!std::isnan(centroid.Y())) {
            y.push_back(centroid.Y());
            zy.push_back(centroid.Z());
         }
      }
    }
  }

  // Need at least 3 measurements
  if (x.size() < 2 || y.size() < 2 || zx.size() < 2 || zy.size() < 2) {
    return {1., 1.};
  }
  double lambda1, lambda2;
  std::tie(lambda1, lambda2) = PCA(x, zx);
  double anisotropy_xz = (lambda1 > 0) ? 1.0 - lambda2 / lambda1 : 0.0;
  auto pca_result_yz = PCA(y, zy);
  std::tie(lambda1, lambda2) = PCA(y, zy);
  double anisotropy_yz = (lambda1 > 0) ? 1.0 - lambda2 / lambda1 : 0.0;
  return {anisotropy_xz, anisotropy_yz};
}

double snd::analysis_tools::GetUSSpatialAnisotropy(const std::vector<USPlane> &us_planes, bool use_all_centroids)
{
  std::vector<double> y, zy;
  for (auto &p : us_planes) {
    // Add measurements per projection and in the respective coordinate vectors
    if (use_all_centroids == false) {
      for (auto &hit : p.GetHits()) {
        y.push_back(hit.y);
        zy.push_back(hit.z);
      }
    } else {
      auto centroid = p.GetCentroid();
      // Check that centroid is valid
      if (!(std::isnan(centroid.Z()) || std::isnan(centroid.Y()))) {
         y.push_back(centroid.Y());
         zy.push_back(centroid.Z());
      }
    }
  }

  // Need at least 3 measurements
  if (y.size() < 2 || zy.size() < 2) {
    return 1.;
  }
  double lambda1, lambda2;
  std::tie(lambda1, lambda2) = PCA(y, zy);
  return (lambda1 > 0) ? 1.0 - lambda2 / lambda1 : 0.0;
}
