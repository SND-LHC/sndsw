#!/usr/bin/env python
import ROOT,os
from rootpyPickler import Pickler
from rootpyPickler import Unpickler
import shipunit as u
import saveBasicParameters
from XRootD import client

theClient = client.FileSystem('root://eospublic.cern.ch')
commonPath = "/eos/experiment/sndlhc/convertedData/physics/"

'''supportedGeoFiles = ["geofile_sndlhc_TI18_V1_06July2022.root","geofile_sndlhc_TI18_V2_12July2022.root","geofile_sndlhc_TI18_V3_08August2022.root",
                     "geofile_sndlhc_TI18_V4_10August2022.root","geofile_sndlhc_TI18_V5_14August2022.root","geofile_sndlhc_TI18_V6_08October2022.root",
                     "geofile_sndlhc_TI18_V7_22November2022.root"]
'''
supportedGeoFiles = {"geofile_sndlhc_TI18_V7_22November2022.root":commonPath+"commissioning/TI18",
                     "geofile_sndlhc_TI18_V0_2024.root":commonPath+"2024/"}

def modifyDicts(year=2024):
   for geoFileName in supportedGeoFiles:
         if (year == 2024 and geoFileName.find(str(year)) > 0) or \
            (year != 2024 and geoFileName.find('2022') > 0):
            # override locally existing file with the same name
            status = theClient.copy(supportedGeoFiles[geoFileName]+geoFileName,
                                    os.getcwd()+'/'+geoFileName, force=True)
            if not status[0].ok: 
               print('Bad status of the local copy process', status)
         else: continue
         fg  = ROOT.TFile(geoFileName)
         pkl = Unpickler(fg)
         sGeo = pkl.load('ShipGeo')
         fg.Close()

         # DS part
         setattr(sGeo.MuFilter,'DsPropSpeed',14.9*u.cm/u.nanosecond)
         sGeo.MuFilter['DsPropSpeed'] = 14.9*u.cm/u.nanosecond
         constants = {}
         constants['t_0'] =  [0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0]
         constants['t_4361'] =  [-5.61,-5.63,-5.90,-5.39,-5.40,-5.58,-5.99,-6.08,-6.27,-6.43,-5.94,-6.20,-5.45,-5.52,-5.75,-5.93,-5.40,-5.56,-5.46,-5.74]
         constants['t_5117'] =  [-6.56,-6.53,-6.75,-6.27,-6.34,-6.46,-7.50,-7.60,-7.79,-7.94,-7.51,-7.72,-8.09,-8.15,-8.38,-8.55,-8.04,-8.14,-8.02,-8.24]
         constants['t_5478'] =  [-5.28,-5.38,-5.71,-5.76,-5.28,-5.46,-4.22,-4.38,-4.55,-4.70,-4.27,-4.46,-5.31,-5.43,-5.71,-5.88,-5.34,-5.45,-5.38,-5.57]
         constants['t_6208'] =  [-6.00,-5.92,-6.33,-6.22,-5.94,-6.15,-5.53,-5.49,-5.81,-5.86,-5.53,-5.77,-7.74,-7.71,-8.01,-8.04,-7.68,-7.82,-7.65,-7.87]
         # 2024 and later on
         constants['t_7648'] =  [-6.53,-6.68,-6.75,-6.81,-6.37,-6.52,-5.81,-5.97,-6.19,-6.31,-5.90,-6.10,-8.06,-8.17,-8.38,-8.54,-8.06,-8.15,-8.07,-8.29]
         constants['t_8318'] =  [-6.52,-6.60,-6.69,-6.73,-6.37,-6.53,-5.84,-5.97,-6.20,-6.34,-5.96,-6.17,-8.10,-8.19,-8.35,-8.51,-8.10,-8.22,-8.11,-8.33]
         constants['t_8582'] =  [-6.60,-6.68,-6.78,-6.80,-6.46,-6.57,-6.01,-6.14,-6.23,-6.37,-6.02,-6.23,-8.15,-8.25,-8.44,-8.60,-8.21,-8.30,-8.29,-8.49]
         slopes_dict_2022 = {"t_0":0.000, "t_4361":0.082, "t_5117":0.085, "t_5478":0.082, "t_6208":0.086}
         # new constants for 2024 and later on
         slopes_dict_2024 = {"t_0":0.000, "t_7648":0.085, "t_8318":0.086, "t_8582":0.085}
         if year == 2024: slopes_dict = slopes_dict_2024
         else: slopes_dict = slopes_dict_2022
         #time delay corrections first order, only for DS at the moment
         for p in slopes_dict.keys():
            setattr(sGeo.MuFilter,'DSTcorslope'+p,slopes_dict[p])
            sGeo.MuFilter['DSTcorslope'+p] = slopes_dict[p]
            for i in range(len(constants[p])): 
               setattr(sGeo.MuFilter,'DSTcorC'+str(i)+p,constants[p][i])
               sGeo.MuFilter['DSTcorC'+str(i)+p] = constants[p][i]

       # Scifi part
         constants={}
         constants['t_0']=[   0.000*u.ns,  0.000*u.ns,  0.000*u.ns, 0.000*u.ns, 0.000*u.ns,  0.000*u.ns, 0.000*u.ns,
  0.000*u.ns,  0.000*u.ns,   0.000*u.ns, 0.000*u.ns, 0.000*u.ns,  0.000*u.ns, 0.000*u.ns,
  0.000*u.ns,  0.000*u.ns,  0.000*u.ns,  0.000*u.ns,  0.000*u.ns,  0.000*u.ns,  0.000*u.ns,
   0.000*u.ns,  0.000*u.ns,  0.000*u.ns,  0.000*u.ns, 0.000*u.ns,  0.000*u.ns, 0.0000*u.ns,
   0.000*u.ns,  0.000*u.ns,  0.000*u.ns, 0.000*u.ns, 0.000*u.ns,  0.000*u.ns,  0.000*u.ns ]
         constants['t_4361']=[   0.000*u.ns,  0.000*u.ns,  -0.222*u.ns, -0.509*u.ns, -0.517*u.ns,  -1.156*u.ns, -0.771*u.ns,
  -0.287*u.ns,  0.000*u.ns,   0.250*u.ns, -0.854*u.ns, -1.455*u.ns,  -0.812*u.ns, -1.307*u.ns,
  -0.861*u.ns,  0.000*u.ns,  -0.307*u.ns,  0.289*u.ns,  0.069*u.ns,  -0.895*u.ns,  0.731*u.ns,
   0.164*u.ns,  0.000*u.ns,  -1.451*u.ns,  0.196*u.ns, -2.025*u.ns,  -1.049*u.ns, -0.938*u.ns,
   0.337*u.ns,  0.000*u.ns,  -1.157*u.ns, -1.060*u.ns, -0.627*u.ns,  -2.405*u.ns,  0.071*u.ns ]
         constants['t_5117']=[   0.000*u.ns,  0.000*u.ns,  -0.325*u.ns, -0.497*u.ns,  0.228*u.ns,  -0.368*u.ns,  0.076*u.ns,
  -0.906*u.ns,  0.000*u.ns,   0.259*u.ns, -0.775*u.ns, -0.370*u.ns,   0.243*u.ns, -0.284*u.ns,
  -0.995*u.ns,  0.000*u.ns,  -0.133*u.ns,  0.473*u.ns,  0.753*u.ns,  -0.433*u.ns,  1.106*u.ns,
  -0.371*u.ns,  0.000*u.ns,  -1.331*u.ns,  0.319*u.ns, -1.979*u.ns,  -1.120*u.ns, -0.981*u.ns,
  -1.276*u.ns,  0.000*u.ns,  -0.755*u.ns, -0.630*u.ns,  0.886*u.ns,  -0.873*u.ns,  1.639*u.ns ]
         constants['t_5478']=[   0.000*u.ns,  0.000*u.ns,  -0.309*u.ns,  -0.591*u.ns,  -0.502*u.ns,  -1.164*u.ns,  -0.753*u.ns, 
  -0.369*u.ns,  0.000*u.ns,  0.239*u.ns,  -0.815*u.ns,   -1.138*u.ns,  -0.578*u.ns,  -1.054*u.ns, 
  -1.011*u.ns,  0.000*u.ns,  -0.270*u.ns,  0.447*u.ns,   0.760*u.ns,  -0.512*u.ns,  0.566*u.ns, 
   0.024*u.ns,  0.000*u.ns,  -1.348*u.ns,  0.283*u.ns,   -1.495*u.ns,  -0.559*u.ns,  -0.545*u.ns, 
   0.195*u.ns,  0.000*u.ns,  -1.131*u.ns,  -0.937*u.ns,   -0.391*u.ns,  -2.102*u.ns,  0.365*u.ns ]
         constants['t_6208']=[   0.000*u.ns,  0.000*u.ns,  -0.309*u.ns,  -0.546*u.ns,   0.169*u.ns,  -0.454*u.ns,  -0.116*u.ns, 
  -0.920*u.ns,  0.000*u.ns,  0.238*u.ns,  -0.799*u.ns,   -0.430*u.ns,  0.186*u.ns,  -0.324*u.ns, 
  -1.042*u.ns,  0.000*u.ns,  -0.140*u.ns,  0.515*u.ns,   0.728*u.ns,  -0.572*u.ns,  0.673*u.ns, 
  -0.432*u.ns,  0.000*u.ns,  -1.366*u.ns,  0.326*u.ns,   -2.076*u.ns,  -1.152*u.ns,  -1.075*u.ns, 
  -1.067*u.ns,  0.000*u.ns,  -1.021*u.ns,  -0.873*u.ns,   0.623*u.ns,  -1.094*u.ns,  1.365*u.ns ]
         constants['t_7648']=[     0.000*u.ns,  0.000*u.ns,  -0.299*u.ns,  -0.510*u.ns,   0.217*u.ns,  -0.401*u.ns,  0.016*u.ns,
  -1.194*u.ns,  0.000*u.ns,  0.547*u.ns,  -0.473*u.ns,   -0.080*u.ns,  0.500*u.ns,  0.002*u.ns,
  -1.017*u.ns,  0.000*u.ns,  -0.129*u.ns,  0.513*u.ns,   0.736*u.ns,  -0.466*u.ns,  0.963*u.ns,
  -0.359*u.ns,  0.000*u.ns,  -1.346*u.ns,  0.307*u.ns,   -2.019*u.ns,  -1.096*u.ns,  -1.012*u.ns,
  -1.032*u.ns,  0.000*u.ns,  -1.002*u.ns,  -0.860*u.ns,   0.630*u.ns,  -1.134*u.ns,  1.393*u.ns ]
         constants['t_8318']=[     0.000*u.ns,  0.000*u.ns,  -0.312*u.ns,  -0.498*u.ns,   0.336*u.ns,  -0.291*u.ns,  0.153*u.ns,
  -0.882*u.ns,  0.000*u.ns,  0.277*u.ns,  -0.753*u.ns,   -0.348*u.ns,  0.257*u.ns,  -0.237*u.ns,
  -0.957*u.ns,  0.000*u.ns,  -0.102*u.ns,  0.507*u.ns,   0.786*u.ns,  -0.420*u.ns,  0.987*u.ns,
  -0.365*u.ns,  0.000*u.ns,  -1.311*u.ns,  0.362*u.ns,   -2.152*u.ns,  -0.970*u.ns,  -0.725*u.ns,
  -0.982*u.ns,  0.000*u.ns,  -0.994*u.ns,  -0.895*u.ns,   0.454*u.ns,  -1.216*u.ns,  1.361*u.ns ]
         constants['t_8582']=[      0.000*u.ns,  0.000*u.ns,  -0.327*u.ns,  -0.494*u.ns,   0.226*u.ns,  -0.377*u.ns,  0.077*u.ns,
  -0.902*u.ns,  0.000*u.ns,  0.270*u.ns,  -0.751*u.ns,   -0.345*u.ns,  0.243*u.ns,  -0.259*u.ns,
  -0.937*u.ns,  0.000*u.ns,  -0.172*u.ns,  0.427*u.ns,   0.696*u.ns,  -0.478*u.ns,  0.916*u.ns,
  -0.355*u.ns,  0.000*u.ns,  -1.380*u.ns,  0.301*u.ns,   -1.998*u.ns,  -1.108*u.ns,  -1.020*u.ns,
  -1.021*u.ns,  0.000*u.ns,  -0.994*u.ns,  -0.892*u.ns,   0.591*u.ns,  -1.051*u.ns,  1.368*u.ns ]
#
         scifi_time_tags_2022 = ['t_0', 't_4361','t_5117', 't_5478', 't_6208']
         scifi_time_tags_2024 = ['t_0', 't_7648', 't_8318', 't_8582']
         if year == 2024: scifi_time_tags = scifi_time_tags_2024
         else: scifi_time_tags = scifi_time_tags_2022
         for c in scifi_time_tags:
          k=0
          for s in range(1,6):
            setattr(sGeo.Scifi,'station'+str(s)+c,constants[c][k])
            sGeo.Scifi['station'+str(s)+c] = constants[c][k]
            k+=1
            for p in ['H0','H1','H2','V0','V1','V2']:
               setattr(sGeo.Scifi,'station'+str(s)+p+c,constants[c][k])
               sGeo.Scifi['station'+str(s)+p+c] = constants[c][k]
               k+=1

         alignment={}
         alignment['t_0']=[
    0.00*u.um,  0.00*u.um,  0.00*u.um,
 0.00*u.um, 0.00*u.um,    0.00*u.um,
 0.00*u.um,  0.00*u.um,  0.00*u.um,
  0.00*u.um,  0.00*u.um,  0.00*u.um,
   0.00*u.um,   0.00*u.um,   0.00*u.um,
    0.00*u.um, 0.00*u.um,   0.00*u.um,
  0.00*u.um,   0.00*u.um,   0.00*u.um,
   0.00*u.um,  0.00*u.um,   0.00*u.um,
   0.00*u.um,   0.00*u.um,  0.00*u.um,
  0.00*u.um,  0.00*u.um,   0.00*u.um,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad]
         alignment['t_4361']=[
    7.30*u.um,  219.99*u.um,  247.73*u.um,
 -103.87*u.um, -105.64*u.um,    2.54*u.um,
 -286.76*u.um,  -53.99*u.um,  -85.45*u.um,
  103.99*u.um,  113.92*u.um,  148.52*u.um,
   -1.85*u.um,   78.98*u.um,   13.98*u.um,
    0.76*u.um, -109.75*u.um,   74.54*u.um,
  -16.79*u.um,   56.44*u.um,   96.94*u.um,
   71.04*u.um,  -64.13*u.um,   17.25*u.um,
   76.32*u.um,   51.34*u.um,  -13.33*u.um,
  -78.20*u.um,  158.73*u.um,   39.76*u.um,
   0.00*u.mrad,   -1.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad]
         alignment['t_4575']=[
  316.44*u.um,  521.82*u.um,  518.38*u.um,
 -226.06*u.um, -103.68*u.um,   65.08*u.um,
 -157.97*u.um,   59.67*u.um,  -12.36*u.um,
  -70.05*u.um,   63.72*u.um,   78.74*u.um,
   -1.85*u.um,   78.98*u.um,   13.98*u.um,
    0.76*u.um, -109.75*u.um,   74.54*u.um,
  -16.79*u.um,   56.44*u.um,   96.94*u.um,
   71.04*u.um,  -64.13*u.um,   17.25*u.um,
  172.36*u.um,   79.96*u.um,   57.20*u.um,
 -128.66*u.um,  104.65*u.um,  -31.24*u.um,
 0.00*u.mrad,   -1.17*u.mrad,    0.00*u.mrad,
 0.00*u.mrad,   -0.47*u.mrad,    0.00*u.mrad,
 0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
 0.00*u.mrad,   -0.28*u.mrad,    0.00*u.mrad,
 0.00*u.mrad,   -0.33*u.mrad,    0.00*u.mrad]
         alignment['t_4855']=[
  513.31*u.um,  775.65*u.um,  629.35*u.um,
 -339.98*u.um,  133.28*u.um,   83.79*u.um,
  162.48*u.um,  395.88*u.um,  297.98*u.um,
 -135.61*u.um,  144.02*u.um,   86.72*u.um,
   -1.85*u.um,   78.98*u.um,   13.98*u.um,
    0.76*u.um, -109.75*u.um,   74.54*u.um,
  -16.79*u.um,   56.44*u.um,   96.94*u.um,
   71.04*u.um,  -64.13*u.um,   17.25*u.um,
   83.73*u.um,   14.94*u.um,   33.91*u.um,
 -173.09*u.um, -184.25*u.um,   -8.56*u.um,
   0.00*u.mrad,   -1.68*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,   -0.68*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.08*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,   -0.37*u.mrad,    0.00*u.mrad]
         alignment['t_5172']=[
   331.99*u.um,  560.17*u.um,  460.83*u.um,
    13.37*u.um,  -57.78*u.um,  230.51*u.um,
  -136.69*u.um,   63.70*u.um,  -30.45*u.um,
     7.77*u.um,   -5.01*u.um,  156.92*u.um,
    -1.85*u.um,   78.98*u.um,   13.98*u.um,
     0.76*u.um, -109.75*u.um,   74.54*u.um,
   -16.79*u.um,   56.44*u.um,   96.94*u.um,
    71.04*u.um,  -64.13*u.um,   17.25*u.um,
    26.80*u.um,   51.14*u.um,  -31.93*u.um,
  -343.13*u.um,  -60.93*u.um, -260.49*u.um,
    0.00*u.mrad,   -1.00*u.mrad,    0.00*u.mrad,
    0.00*u.mrad,   -0.59*u.mrad,    0.00*u.mrad,
    0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
    0.00*u.mrad,    0.35*u.mrad,    0.00*u.mrad,
    0.00*u.mrad,   -0.32*u.mrad,    0.00*u.mrad]
         alignment['t_5431']=[       # 2023 emulsion run 4
   458.16*u.um,  658.14*u.um,  569.77*u.um,
  -285.21*u.um, -215.62*u.um,  -10.06*u.um,
  -131.42*u.um,   61.04*u.um,   18.21*u.um,
   -55.62*u.um,  -44.88*u.um,   60.99*u.um,
   -1.85*u.um,   78.98*u.um,   13.98*u.um,
    0.76*u.um, -109.75*u.um,   74.54*u.um,
  -16.79*u.um,   56.44*u.um,   96.94*u.um,
   71.04*u.um,  -64.13*u.um,   17.25*u.um,
   588.09*u.um,  500.82*u.um,  306.64*u.um,
  -429.88*u.um, -350.11*u.um, -408.64*u.um,
   0.00*u.mrad,   -3.06*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,   -1.34*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,    0.81*u.mrad,    0.00*u.mrad,
   0.00*u.mrad,   -0.30*u.mrad,    0.00*u.mrad]
         alignment['t_6305']=[       # 2023 emuslsion run 5
   374.80*u.um, 468.49*u.um, 564.50*u.um,
  -130.44*u.um, -378.11*u.um, -29.46*u.um,
  -163.03*u.um, -78.21*u.um, -7.67*u.um,
   9.35*u.um, -124.07*u.um, 102.21*u.um,
  -1.85*u.um, 78.98*u.um, 13.98*u.um,
   0.76*u.um, -109.75*u.um, 74.54*u.um,
  -16.79*u.um, 56.44*u.um, 96.94*u.um,
   71.04*u.um, -64.13*u.um, 17.25*u.um,
   579.12*u.um, 576.12*u.um, 502.01*u.um,
  -581.49*u.um, -220.94*u.um, -470.78*u.um,
   0.00*u.mrad, -3.86*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, -1.77*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.00*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.88*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.07*u.mrad, 0.00*u.mrad]
# 2024
         alignment['t_7648']=[       # 2024 emuslsion run 8
   509.86*u.um, 732.95*u.um, 621.19*u.um,
  -10.56*u.um, 153.93*u.um, 92.22*u.um,
   94.54*u.um, 342.84*u.um, 168.64*u.um,
   164.88*u.um, 397.75*u.um, 246.37*u.um,
   185.00*u.um, 430.61*u.um, 245.85*u.um,
   0.76*u.um, 175.09*u.um, 166.33*u.um,
   0.00*u.um, 274.57*u.um, 185.82*u.um,
   50.00*u.um, 249.06*u.um, 175.30*u.um,
   313.78*u.um, 526.78*u.um, 361.02*u.um,
  -267.73*u.um, 339.17*u.um, 37.59*u.um,
   0.00*u.mrad, -0.61*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.34*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.00*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.20*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, -0.55*u.mrad, 0.00*u.mrad]
         alignment['t_8318']=[       # 2024 emuslsion run 9
   469.66*u.um, 611.18*u.um, 693.42*u.um,
   201.35*u.um, 229.61*u.um, 103.33*u.um,
   60.22*u.um, 322.78*u.um, 264.75*u.um,
   496.09*u.um, 643.18*u.um, 472.68*u.um,
   234.71*u.um, 585.04*u.um, 382.37*u.um,
   529.26*u.um, 650.01*u.um, 614.34*u.um,
   431.02*u.um, 905.43*u.um, 748.38*u.um,
   116.27*u.um, 335.47*u.um, 201.41*u.um,
   100.54*u.um, 600.22*u.um, 269.97*u.um,
   550.18*u.um, 1248.48*u.um, 931.66*u.um,
   0.00*u.mrad, -3.02*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, -0.95*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.12*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, -1.55*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 2.31*u.mrad, 0.00*u.mrad]
         alignment['t_8582']=[       # 2024 emuslsion run 10
   611.7976*u.um, 889.86*u.um, 879.801*u.um,
   413.6267*u.um, 363.6332*u.um, 105.825*u.um,
   127.2448*u.um, 487.3149*u.um, 364.0717*u.um,
   707.2705*u.um, 849.2645*u.um, 589.3346*u.um,
   271.2779*u.um, 655.8296*u.um, 414.7906*u.um,
   686.1134*u.um, 851.719*u.um, 798.134*u.um,
   135.175*u.um, 614.5551*u.um, 442.9096*u.um,
   821.2312*u.um, 1189.1332*u.um, 1063.688*u.um,
   255.8865*u.um, 731.2859*u.um, 392.1363*u.um,
   507.8946*u.um, 1421.0582*u.um, 1139.182*u.um,
   0.00*u.mrad, -3.49*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, -1.13*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 0.10*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 1.44*u.mrad, 0.00*u.mrad,
   0.00*u.mrad, 2.09*u.mrad, 0.00*u.mrad]

         scifi_spatial_tags_2022 = ['t_0', 't_4361','t_4575','t_4855','t_5172','t_5431', 't_6305']
         scifi_spatial_tags_2024 = ['t_0', 't_7648', 't_8318', 't_8582']
         if year == 2024: scifi_spatial_tags = scifi_spatial_tags_2024
         else: scifi_spatial_tags = scifi_spatial_tags_2022
         for c in scifi_spatial_tags:
          k=0
          for s in range(1,6):
           for o in range(0,2):
             for m in range(3):
               nr = s*100+10*o+m
               setattr(sGeo.Scifi,'LocM'+str(nr)+c,alignment[c][k])
               sGeo.Scifi['LocM'+str(nr)+c] = alignment[c][k]
               k+=1
          for s in range(1,6):
           for o in ["RotPhiS","RotPsiS","RotThetaS"]:
               setattr(sGeo.Scifi,o+str(s)+c,alignment[c][k])
               sGeo.Scifi[o+str(s)+c] = alignment[c][k]
               k+=1

         print('save',geoFileName)
         saveBasicParameters.execute(geoFileName,sGeo)
         

