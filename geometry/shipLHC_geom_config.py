import ROOT as r
import shipunit as u
from ShipGeoConfig import AttrDict, ConfigRegistry


with ConfigRegistry.register_config("basic") as c:
# cave parameters
        c.cave = AttrDict(z=0*u.cm)

        c.EmulsionDet = AttrDict(z=0*u.cm)
        c.EmulsionDet.PassiveOption = 1 #0 makes emulsion volumes active, 1 makes all emulsion volumes passive
        c.EmulsionDet.row = 2
        c.EmulsionDet.col = 2
        c.EmulsionDet.wall= 5
        c.EmulsionDet.target = 1  #number of neutrino target volumes
        c.EmulsionDet.n_plates = 59
        c.EmulsionDet.EmTh = 0.0070 * u.cm
        c.EmulsionDet.EmX = 19.2 * u.cm
        c.EmulsionDet.EmY = 19.2 * u.cm
        c.EmulsionDet.PBTh = 0.0175 * u.cm
        c.EmulsionDet.PassiveTh = 0.1 * u.cm
        c.EmulsionDet.EPlW = 2* c.EmulsionDet.EmTh + c.EmulsionDet.PBTh
        c.EmulsionDet.AllPW = c.EmulsionDet.PassiveTh + c.EmulsionDet.EPlW

        c.EmulsionDet.BrPackZ = 0.*u.cm
        c.EmulsionDet.BrPackX = 2*0.3*u.cm
        c.EmulsionDet.BrPackY = 2*0.3*u.cm
        
        c.EmulsionDet.BrX = c.EmulsionDet.EmX + c.EmulsionDet.BrPackX
        c.EmulsionDet.BrY = c.EmulsionDet.EmY + c.EmulsionDet.BrPackY
        c.EmulsionDet.BrZ = c.EmulsionDet.n_plates * c.EmulsionDet.AllPW + c.EmulsionDet.EPlW + c.EmulsionDet.BrPackZ

        c.EmulsionDet.xdim = c.EmulsionDet.BrX * c.EmulsionDet.col
        c.EmulsionDet.ydim = c.EmulsionDet.BrY * c.EmulsionDet.row
        c.EmulsionDet.WallXDim = c.EmulsionDet.xdim
        c.EmulsionDet.WallYDim = c.EmulsionDet.ydim
        c.EmulsionDet.WallZDim = c.EmulsionDet.BrZ
        c.EmulsionDet.TTz = 3.0*u.cm
        c.EmulsionDet.zdim = c.EmulsionDet.wall* c.EmulsionDet.WallZDim + c.EmulsionDet.wall*c.EmulsionDet.TTz
        c.EmulsionDet.ShiftX = -8.0*u.cm - c.EmulsionDet.xdim/2.
        c.EmulsionDet.ShiftY = 15.5*u.cm + c.EmulsionDet.ydim/2.

        c.EmulsionDet.startpos =  -25.4750 * u.cm
        c.EmulsionDet.zC = c.EmulsionDet.startpos + c.EmulsionDet.zdim/2.

        c.Scifi = AttrDict(z=0*u.cm)
        c.Scifi.xdim = c.EmulsionDet.xdim
        c.Scifi.ydim = c.EmulsionDet.ydim
        c.Scifi.zdim = c.EmulsionDet.TTz
        c.Scifi.DZ = c.EmulsionDet.BrZ
        c.Scifi.nplanes = c.EmulsionDet.wall

        c.MuFilter = AttrDict(z=0*u.cm)
        #coordinates in local gravity based system, not complete, for testing interface
        c.MuFilter.Veto1 = AttrDict(Dz=-151.312*u.mm)
        c.MuFilter.Veto1.Dx = -44.108*u.mm
        c.MuFilter.Veto1.Dy = 2800.662*u.mm
        c.MuFilter.Veto2 = AttrDict(Dz=-130.804*u.mm)
        c.MuFilter.Veto2.Dx = -44.108*u.mm
        c.MuFilter.Veto2.Dy = 2841.41*u.mm
        c.MuFilter.Iron1 = AttrDict(Dz=-102.137*u.mm)
        c.MuFilter.Iron1.Dx = 27.958*u.mm
        c.MuFilter.Iron1.Dy = 3581.523*u.mm
        c.MuFilter.Muon1 = AttrDict(Dz=-99.894*u.mm)
        c.MuFilter.Muon1.Dx = 51.07*u.mm
        c.MuFilter.Muon1.Dy = 3762.027*u.mm


        #Veto station parameters
        c.MuFilter.VetonSiPMs = 8
        c.MuFilter.VetonSides = 2
        c.MuFilter.NVetoPlanes = 2
        c.MuFilter.VetoShiftX = c.EmulsionDet.ShiftX
        c.MuFilter.VetoShiftY = c.EmulsionDet.ShiftY
        c.MuFilter.VetoPlaneShiftY = 1*u.cm
        
        c.MuFilter.VetoPlaneX = 42 *u.cm
        c.MuFilter.VetoPlaneY = 42 *u.cm
        c.MuFilter.VetoPlaneZ = 4 * u.cm

        c.MuFilter.NVetoBars = 7

        c.MuFilter.VetoBarX = c.MuFilter.VetoPlaneX
        c.MuFilter.VetoBarY = c.MuFilter.VetoPlaneY / c.MuFilter.NVetoBars
        c.MuFilter.VetoBarZ = 1 * u.cm

        #veto should end at the start of first ECC target
        c.MuFilter.VetozC = c.EmulsionDet.zC - c.EmulsionDet.zdim/2. - (c.MuFilter.NVetoPlanes * c.MuFilter.VetoPlaneZ)/2.

        #c.MuFilter.X = c.EmulsionDet.xdim + 20*u.cm
        c.MuFilter.X = 80.0*u.cm
        #c.MuFilter.Y = c.EmulsionDet.ydim + 20*u.cm+10.0*u.cm
        c.MuFilter.Y = 60.0*u.cm
        c.MuFilter.FeX = c.MuFilter.X
        #c.MuFilter.FeY = c.EmulsionDet.ydim + 20*u.cm
        c.MuFilter.FeY = c.MuFilter.Y
        c.MuFilter.FeZ = 20*u.cm
        c.MuFilter.UpstreamDetX = c.MuFilter.X
        c.MuFilter.UpstreamDetY = c.MuFilter.FeY
        c.MuFilter.UpstreamDetZ = 2.6*u.cm
        c.MuFilter.UpstreamnSiPMs = 8
        c.MuFilter.UpstreamnSides = 2
        c.MuFilter.NUpstreamPlanes = 5
        c.MuFilter.DownstreamDetX = c.MuFilter.X
        c.MuFilter.DownstreamDetY = c.MuFilter.FeY
        c.MuFilter.DownstreamDetZ = 3.9*u.cm
        c.MuFilter.DownstreamnSiPMs = 1
        c.MuFilter.DownstreamnSides = 2   # only for horizontal, vertical only one side
        c.MuFilter.NDownstreamPlanes = 4
        c.MuFilter.DS4ZGap = 8.82*u.cm
        #upstream bars configuration
        c.MuFilter.NUpstreamBars = 10
        c.MuFilter.UpstreamBarX = c.MuFilter.UpstreamDetX
        c.MuFilter.UpstreamBarY = c.MuFilter.UpstreamDetY/c.MuFilter.NUpstreamBars #computed for staggering
        c.MuFilter.UpstreamBarZ = 1*u.cm

        #downstream bars configuration
        c.MuFilter.NDownstreamBars = 60 #n.d.r. both for x and y in this case
        c.MuFilter.DownstreamBarX = c.MuFilter.DownstreamDetX
        c.MuFilter.DownstreamBarY = c.MuFilter.DownstreamDetY/c.MuFilter.NDownstreamBars #computed for staggering
        c.MuFilter.DownstreamBarZ = 1*u.cm

        c.MuFilter.DownstreamBarX_ver = c.MuFilter.DownstreamDetY/c.MuFilter.NDownstreamBars #the vertical bars cover a region only 60 x 60 cm2
        c.MuFilter.DownstreamBarY_ver = 63.5*u.cm 
        c.MuFilter.DownstreamBarZ_ver = 1*u.cm

        #total z thickness and position
        c.MuFilter.Z = c.MuFilter.NUpstreamPlanes*(c.MuFilter.FeZ+c.MuFilter.UpstreamDetZ) + (c.MuFilter.NDownstreamPlanes - 1)*(c.MuFilter.FeZ+c.MuFilter.DownstreamDetZ) + c.MuFilter.DS4ZGap + c.MuFilter.DownstreamDetZ/2 #doesn't include veto
        c.MuFilter.Zcenter = c.EmulsionDet.zC+c.EmulsionDet.zdim/2+c.MuFilter.Z/2
        c.MuFilter.ShiftX = -2.8 * u.cm - c.MuFilter.X/2.
        
        c.MuFilter.Slope = -3.2 #in degrees
        c.MuFilter.ShiftY = 9.6 * u.cm + c.MuFilter.Y/2. #shift of first block of upstream section
        c.MuFilter.ShiftYEnd= 7.5*u.cm + c.MuFilter.Y/2. #shift for downstream section

        #digitization parameters
        c.MuFilter.AttenuationLength = 28 * u.cm # Bologna prototype of DS bar
        c.MuFilter.SiPMcalibration         = 0.208        # Volt/Mev Bologna prototype, this for Amplitude, 200 V/MeV for QDC
        c.MuFilter.DynRangeLow           = 0.01       # just for test, don't know realistic value
        c.MuFilter.DynRangeHigh          = 1.0         # just for test, don't know realistic value

        c.Floor = AttrDict(z=483262./10.*u.cm)   # Antonia, 482000mm (FASER+2, P3) + 1017mm (DZ) + 245mm (centre emulsion)
