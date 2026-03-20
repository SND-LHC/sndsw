# Changelog

All notable changes to this project will be documented in this file.
The latest changes are placed on top get assigned to the corresponding
software release.

The software versioning scheme is vMAJOR.MINOR.PATCH+DATE-comment, where
 - PATCH - quick bug fixes
 - MINOR - new classes or functionality, updates to adapt to new external packages
 - MAJOR - additions of the detectors, breaking changes - hopefully we won't have such
 - DATE - to ease communication and mimic our CVMFS releases
 - comment - a short description of changes if available
 - e.g. v1.0.0+2025-07-updateScifi


The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
There could be several types of changes:
- `Added` for new features.
- `Changed` for changes in existing functionality.
- `Deprecated` for soon-to-be removed features.
- `Removed` for now removed features.
- `Fixed` for any bug fixes.
- `Security` in case of vulnerabilities.

We start with the first sndsw release: v1.0.0+2025-07-updateScifi.
Shall there be a strong will/need, one can go back and create and
fill in the logs for previous stacks.

## v1.4.0+2026-01-G4cutoff

### Added

- Geant4 range cut set to 2mm
- Record simulation run and event ID
- `is_mc` option for sndConfiguration, used in all sndPlane classes
-  Veto and DS plane classes
- 2025 data file locations for the analysis tools

### Changed

- Geant4 physics list to FTFP_BERT_HP_EMZ
-  retire small SiPMs: add a flag to skip(default) them in analysis

### Removed

- Geant4 production cut at 1MeV
- hard coded digi params in run_digi macro

### Fixed

- MeV to qdc conversion for US
- Scifi station 5 rotation for target 256
- revised the drift velocity and attenuation length of Veto/US/DS using 2022-2025 data


## v1.3.0+2025-11-showerToolsAndP8Decayer

### Added

- shower tools: start, stop, shower direction
- 2dED: Add option to plot shower direction
- python script to make run lists with DB info
- SciFi and US anisotropy tools
- USPlane: GetNHitBars() returting N bars with fired SiPMs above threshold
- MuFilterHit: Add GetImpactXpos method
- add time window selection for US(test_beam_2023 configuration)
- alignment of targets 256-258

### Changed

- G4conf: Remove obsolete neutron and hadronic process verbosity settings
- improve centroid error: use propagation of uncertainty respecting the weight(=qdc) per hit.
- USPlane: Utilize left/right measurements to determine X position along bar
- USPlane: use only small SiPMs to determine if plane HasShower()
- set default csv tables for geofiles and data ROOT files
    - these changes in are not backward-compatible, but only affect analysis code that is not part of sndsw 
- remove invalid SciFi hits from ScifiPlane's constructor
- 2dED: Plot tracks starting from most upstream Veto

### Fixed

- change pythia8 decayer configuration method
    - heavy mesons and taus are decay’ed using external Pythia8 decayer
- USPlane: Fix FindCentroid method
- Fix reference SciFi time intervals to values used in prev analysis
- MuFilterHit: fix GetImpactT method
- multiple fixes due to drop of attribute pythonization for TDirectory and TClonesArray
- Disable web-viewer when checking for geo overlaps (workaround!)

## v1.2.1+2025-09

### Added

- alignment of target 255

### Changed

- optimize the MCEventBuider:
    - break loop over chunks after first one if using option saveFirst25nsOnly
    - in NC neutrino events, the outgoing neutrino is now saved in the first chunk

### Fixed

- fix in SciFi and US plane classes to allow channel indexing in MC and data alike

## v1.2.0+2025-09-MCEventBuilder

### Added

- MCEventBuilder Fair Task

## v1.1.0+2025-08-BolognaTools

### Added

- analysys classes for SciFi and US planes, together with parameter configuraions for TI18 and testbeams setups
- SciFi tools: hit density

## v1.0.1+2025-07-fixTB24Wtarget

### Fixed

- flag logic to select target geometry for testbeam 2024
- unit fix for selectScifiHits's `time_lower_range` default

### Added

- options for the 2dEvent display: filter hits in time, plot collision axis and more.

## v1.0.0+2025-07-updateScifi

### Added

- CHANGELOG
- Data file and geometry getters from run number

### Changed

- Major update on the TI18 and baby SciFi detector geometry
- Update on the testbeams system geometry to take care of air gaps

### Fixed

- Units for energy and snd_TDC2ns in the cpp units header
- storage of Emulsion(using a flag) and Veto points(always)
- Adapted to the latest FairRoot v19

### Removed

- TI18 MC geometry config file is gone. We use a single one for data and simulation
