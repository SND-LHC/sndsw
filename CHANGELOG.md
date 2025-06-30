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
