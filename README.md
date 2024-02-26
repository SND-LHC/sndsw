sndsw
=====

<!-- markdown-toc start - Don't edit this section. Run M-x markdown-toc-refresh-toc -->
**Table of Contents**

- [Introduction](#introduction)
    - [Contact and communication](#contact-and-communication)
    - [Branches](#branches)
- [Build instructions](#build-instructions)
    - [On lxplus or systems with CVMFS](#on-lxplus-or-systems-with-cvmfs)
    - [On systems without access to CVMFS](#on-systems-without-access-to-cvmfs)
- [Run Instructions](#run-instructions)
- [Development](#development) 
    - [How to keep branches up to date](#how-to-keep-branches-up-to-date)
    - [How to contribute code](#how-to-contribute-code)
- [Docker Instructions](#docker-instructions)

<!-- markdown-toc end -->

# Introduction

`sndsw` is the software framework of the SND@LHC collaboration. It is based on
the [FairShip](https://github.com/ShipSoft/FairShip) framework developed by the SHiP collaboration which in turn is
based on [FairRoot](https://github.com/FairRootGroup/FairRoot/), making use of the automatic python bindings provided by
PyROOT.

## Contact and communication

If you have questions or problems, please feel free to contact the 
@SND-LHC/core-developers. For troubleshooting and development, we plan to discuss on [Mattermost](https://mattermost.web.cern.ch/sndlhc-daq/channels/software-and-analysis).

The [snd-software](mailto:snd-software@cern.ch) mailing list can be used to discuss the software and report issues. Important annoucements will be made there.

The [snd-software-notifications](mailto:snd-software-notifications@cern.ch) mailing list will be used for automated notifications from Github and CI/CD etc..

Both mailing lists are self-subscribe CERN e-groups.

## Branches

<dl>
  <dt><code>master</code></dt>
  <dd>Main development branch.
      All python code is <b>required to be compatible with 3</b>
      Requires aliBuild default <code>release</code>.</dd>
</dl>

# Build instructions

The `aliBuild` family of tools developed by ALICE is used to set up `sndsw` and
its dependencies.

### Introduction to `aliBuild`

The basic commands are the same regardless of whether CVMFS is used:

<dl>
  <dt><code>aliBuild build &lt;package-name&gt; -c snddist</code></dt>
  <dd>Build the package <code>&lt;package-name&gt;</code> (e.g. <code>sndsw</code>) and its dependencies using the recipes and configuration provided by snddist.
  On CVMFS, it is recommended to add <code>--always-prefer-system</code> to ensure packages are used from CVMFS instead of being rebuilt.</dd>
  <dt><code>aliDoctor &lt;package-name&gt;  -c snddist</code></dt>
  <dd>Provide troubleshooting information and hints on which packages can be used from the system for <code>&lt;package-name&gt;</code> and its dependencies.</dd>
  <dt><code>alienv enter &lt;package-name&gt; /latest -c snddist</code></dt>
  <dd>Enter an environment with <code>&lt;package-name&gt;</code> and its dependencies.</dd>
</dl>

For more information on using `aliBuild`, see its
[documentation](https://alisw.github.io/alibuild/) (note: some things are ALICE
specific and will not apply to SND@LHC software).

## On lxplus or systems with CVMFS

On `lxplus` or any CC7/CC8 machine with access to CVMFS, you can do the following:

1. Make sure you can access the SNDLHC CVMFS Repository
    ```bash
    ls /cvmfs/sndlhc.cern.ch
    ```
2. Source the `setUp.sh` script
    ```bash
    source /cvmfs/sndlhc.cern.ch/SNDLHC-2023/Aug30/setUp.sh  # recommended latest version
    ```
3. If you don't want to modify the sndsw package, skip step 3:
    ```bash
    git clone https://github.com/SND-LHC/sndsw
    ```
   This gives you by default the master branch of the software. In case, you want to use a specific branch:
    ```bash
    cd sndsw
    git checkout <branch>
    cd ..
    ```
4. Build the software using `aliBuild`
    ```bash
    aliBuild build sndsw -c $SNDDIST --always-prefer-system --default release
    ```
If you exit your shell session and you want to go back working on it, make sure to re-execute the second step.

To load the `sndsw` environment, after you build the software, you can simply use:

5. Load the environment
    ```bash
    alienv enter sndsw/latest
    ```

However, this won't work if you are using HTCondor. In such case you can do:

```bash
eval $(alienv load sndsw/latest --no-refresh)
```

If you modify `sndsw`, simply repeat step 4 from `sndsw`'s parent directory.

Note for Ubuntu 20.04: currently there is a version (March 2022) of CVMFS built with Ubuntu 20.04. However, standard ubuntu 20.04 still have python2 has default, while we moved to python3.
So, it is needed to do 
```bash
sudo apt-get install python-is-python3
```

Then, the above procedure works
## On systems without access to CVMFS

Commands are similar to the previous case, but without access to CVMFS you need to build the required packages from source.

1. Clone the [snddist](https://github.com/SND-LHC/snddist), which containts the recipes to build `sndsw` and it's dependencies:
    ```bash
    git clone https://github.com/SND-LHC/snddist.git
    ```
2.  If you don't want to modify the sndsw package, skip step 2:
    ```bash
    git clone https://github.com/SND-LHC/sndsw
    ```
    This gives you by default the master branch of the software. In case, you want to use a specific branch:
    ```bash
    cd sndsw
    git checkout <branch>
    cd ..
    ```
    
3. Install [aliBuild](https://github.com/alisw/alibuild)
    ``` bash
    pip3 install --user alibuild
    ```
    and make sure that it is in your $PATH, or if you are administrator:
    ``` bash
    sudo pip3 install alibuild
    ```

4. Build the software using aliBuild
    ```bash
    aliBuild build sndsw -c snddist --default release
    ```
    If you run into any problems, `aliDoctor` can help determine what the problem is.
5. Load the environment
    ```bash
    alienv enter sndsw/latest
    ```


# Run Instructions

updated  11 October 2021 for the use with raw data

 Set up the bulk of the environment from CVMFS. 

 ```bash 
 source /cvmfs/sndlhc.cern.ch/latest/setUp.sh 
 ``` 

 Load your local sndsw environment. 

 ```bash 
 alienv enter (--shellrc) sndsw/latest 
 ```     

  ```bash 
 python $SNDSW_ROOT/shipLHC/run_simSND.py  --Ntuple  -n 100 -f /eos/experiment/sndlhc/MonteCarlo/FLUKA/muons_up/version1/unit30_Nm.root  --eMin 1.0
 >> Macro finished succesfully. 
 ```
 >> Output files are  sndLHC.Ntuple-TGeant4.root (events) and  geofile_full.Ntuple-TGeant4.root  (setup) 

 Run the event display: 

 ```bash 
 python -i $SNDSW_ROOT/macro/eventDisplay.py -f sndLHC.Ntuple-TGeant4.root -g geofile_full.Ntuple-TGeant4.root 
 // use SHiP Event Display GUI 
 Use quit() or Ctrl-D (i.e. EOF) to exit 
 ``` 
 a) Use the GUI to display  events: SHiP actions / next event

 b) Hoovering over trajectory will display additional information : 
 
 c) At python prompt: sTree.MCTrack.Dump() will display info about all MC particles 

## Use cases covered by `run_simSND.py`:

1. Transport muons, output of FLUKA simulation, to TI18 and the detector. Positive and negative muons, up and down crossing angles, exist.
Possible options are setting minimum energy for transporting particles, transport only muons, increase EM cross sections of muons.

 ```bash 
 python $SNDSW_ROOT/shipLHC/run_simSND.py  --Ntuple  -n nEvents  -f /eos/experiment/sndlhc/MonteCarlo/FLUKA/muons_up/version1/unit30_Nm.root  --eMin ecut
 ```

2. Muon deep inelastic scattering events, produced with pythia6, and then positioned in T18 and transported by Geant4:
 ```bash 
 python  $SNDSW_ROOT/shipLHC/run_simSND.py  -F --MuDIS -n nEvents -f  /eos/experiment/sndlhc/MonteCarlo/Pythia6/MuonDIS/muonDis_1001.root  --eMin ecut
 ```
3. WORK ONGOING: Neutrino events, produced by GENIE, sndsw/macro/makeSNDGenieEvents.py, and then positioned in T18 and transported by Geant4:
 ```bash 
 python  $SNDSW_ROOT/shipLHC/run_simSND.py  --Genie -n nEvents -f ...
 ```

## Digitization of MC data:

1. Convert MC points to detector hits. Input required, data from simulation together with the geometry file created when running simulation. New objects created are `Digi_ScifiHits` together with `Cluster_Scifi` and `Digi_MuFilterHit`, and in parallel objects to make the link to the original MC points, `Digi_MuFilterHits2MCPoints` and `Digi_ScifiHits2MCPoints`.

 ```bash 
 python $SNDSW_ROOT/shipLHC/run_digiSND.py   -f sndLHC.Ntuple-TGeant4.root -g geofile_full.Ntuple-TGeant4.root
 ```

## Converting raw data to sndsw format:

1. Runs the calibration procedure and creates `Digi_ScifiHits` and `Digi_MuFilterHit` with signal and time information from SiPM channels.

 ```bash 
 python $SNDSW_ROOT/shipLHC/rawData/convertRawData.py -p /eos/experiment/sndlhc/testbeam/scifi-cosmic/ -r 35
 ```
2. For the MuFilter testbeam in H8, a specialized script needs to be used to also synchronize the readout boards.

 ```bash 
 python $SNDSW_ROOT/shipLHC/rawData/convertRawData_convertRawData_muTestbeam.py -p /eos/experiment/sndlhc/testbeam/MuFilter/TB_data_commissioning/ -n 5000000  -r 91
 ```
## Example scripts for accessing the raw data and making histograms:
1.  For scifi data:
 ```bash 
 python $SNDSW_ROOT/shipLHC/rawData/scifiHitMaps.py -p /eos/experiment/sndlhc/testbeam/scifi/sndsw/ -r 1 -g geofile_full.Ntuple-TGeant4.root 
 ```

2. For MuFi data:
 ```bash 
 python $SNDSW_ROOT/shipLHC/rawData/mufiHitMaps.py -p /eos/experiment/sndlhc/testbeam/MuFilter/TB_data_commissioning/sndsw/ -r 90 -g geofile_full.Ntuple-TGeant4.root 
 ```
Two methods implemented, hitMaps(Nev = -1) and eventTime().

## simple 2d event display with Scifi tracking:
1. Use method loopEvents(start=0,save=False,goodEvents=False,withTrack=False)
 ```bash 
 python $SNDSW_ROOT/shipLHC/scripts/scifiHitMaps.py -p /eos/experiment/sndlhc/testbeam/scifi/sndsw/ -r 1 -g geofile_full.Ntuple-TGeant4.root 
 ```

# Development

All packages are managed in Git and GitHub. Please either use the web interface to create pull requests or issues, or [send patches via email](https://git-send-email.io/).

If your changes would also benefit [FairShip](https://github.com/ShipSoft/FairShip), please consider making a pull-request for your changes there. We can then pick up the changes from FairShip automatically.

## How to keep branches up to date

To update one's local copy of a branch with the latest modifications of that branch on the upstream repository do:
```bash
cd sndsw
git checkout <branch>
git pull --rebase
cd ..
```
and build the software as usual per the [Build instructions](#build-instructions).

While working on your local copy of a feature `branch`, it is possible that the upstream `master` software changes (e.g. because a pull request was merged into master).
Then one propagates the updates of the upstream `master` to the local `branch` (`master -> branch`) doing:
```bash
cd sndsw
git checkout master
git pull --rebase
git checkout <branch>
git pull --rebase # skip this step if the branch only exists locally
git rebase -i master
cd ..
```
This will put all commits of the feature branch on top of the `master` commits.
Note that before the `rebase` the local feature `branch` was updated via `git pull --rebase` too in case it changed on the upstream repository.

If the `master` and the local `branch` both change the same line(s) in a file(s), a `conflict` arises. Don't panic!
Follow the command-line instructions to resolve them or seek help through the [snd-software](mailto:snd-software@cern.ch) mailing list
or the [Software and Analysis Mattermost channel](https://mattermost.web.cern.ch/sndlhc-daq/channels/software-and-analysis).
One can verify the `commit history` reading the `git log` or inspecting via the graphical tool typing `gitk`.

After solving any encountered `conflict`, build the updated branch as per the [Build instructions](#build-instructions).

Following a `rebase` and after solving `conflicts`, to push one's local changes to the feature `branch` on the upstream repository do:
```bash
git push --force-with-lease --set-upstream origin <branch>
```
Follow the link for further reading why we use [`--force-with-lease` flag](https://stackoverflow.com/questions/52823692/git-push-force-with-lease-vs-force/52823955#52823955).

## How to contribute code

Proposals for contributions to any target branch are made using `Pull requests`.
Furthermore, all `Pull requests` to the `master` branch on the upstream repository go through a review process by the `sndsw` [`core-developers`](https://github.com/orgs/SND-LHC/teams/core-developers).
To make a `Pull request`, follow the guidelines below:

1. Prepare you feature `branch` and commit all your updates/fixes.
2. Update the branch and take care of potential conflicts, as described [above](#update-branch-instructions)
3. Go online to [the sndsw Pull request webpage](https://github.com/SND-LHC/sndsw/pulls) and click on `New pull request`.
   Follow through the fields and verify the request goes from `branch` to the desired branch of `SND-LHC/sndsw`, i.e.
   `base:target_branch <- compare:branch`
4. Leave a message explaining your changes, the need for them, etc.
For `Pull requests` to the `master` branch:
5. On the right panel, under the `Reviewers` heading one can chose who should review the code, e.g. the expert for a syb-system or topic, or someone who you have discussed the changes with.
   Otherwise, the default is `core-developers`.
6. Create the `Pull request`.


# Docker Instructions

Docker is **not** the recommended way to run `sndsw` locally. It is ideal
for reproducing reproducible, stateless environments for debugging, HTCondor
and cluster use, or when a strict separation between `sndsw` and the host is
desirable.

1. Build an docker image from the provided `Dockerfile`:
    ```bash
    git clone https://github.com/SND-LHC/sndsw.git
    cd sndsw
    docker build -t sndsw .
    ``` 
2. Run the `sndsw` docker image:
    ```bash
    docker run -i -t --rm sndsw /bin/bash
    ``` 
3. Advanced docker run options:
    ```bash
    docker run -i -t --rm \
    -e DISPLAY=unix$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /local\_workdir:/image\_workdir \
    sndsw /bin/bash
    ``` 
    The option `-e DISPLAY=unix$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix` forwards graphics from the docker to your local system (similar to `ssh -X`). The option `-v /local_workdir:/image_workdir` mounts `/local_workdir` on the local system as `/image_workdir` within docker.
