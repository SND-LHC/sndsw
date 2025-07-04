import ROOT as r
import shipunit as u
from ShipGeoConfig import AttrDict, ConfigRegistry

if "nuTargetPassive" not in globals():
    nuTargetPassive = True
if "useNagoyaEmulsions" not in globals():
    useNagoyaEmulsions=False
if "year" in globals():
    year = int(globals()["year"])

with ConfigRegistry.register_config("basic") as c:
# cave parameters
        c.cave = AttrDict(z=0*u.cm)
        
        # Antonia, 482000mm (FASER+2, P3) + 1017mm (DZ) + 245mm (centre emulsion),z=483262./10.*u.cm
        # centre emulsion now 326.2cm downstream from origin.
        c.EmulsionDet = AttrDict(z=326.2*u.cm)
        c.EmulsionDet.PassiveOption = nuTargetPassive #0 makes emulsion volumes active, 1 makes all emulsion volumes passive
        c.EmulsionDet.row = 2
        c.EmulsionDet.col = 2
        c.EmulsionDet.wall= 5
        c.EmulsionDet.target = 1  #number of neutrino target volumes
        c.EmulsionDet.n_plates = 59
        c.EmulsionDet.EmTh = 0.0070 * u.cm
        c.EmulsionDet.EmX = 19.2 * u.cm
        c.EmulsionDet.EmY = 19.2 * u.cm
        c.EmulsionDet.PBTh = 0.0175 * u.cm
        if (useNagoyaEmulsions):
                c.EmulsionDet.n_plates = 56
                c.EmulsionDet.PBTh = 0.0195 * u.cm
        
        c.EmulsionDet.PassiveTh = 0.1 * u.cm
        c.EmulsionDet.EPlW = 2* c.EmulsionDet.EmTh + c.EmulsionDet.PBTh
        c.EmulsionDet.AllPW = c.EmulsionDet.PassiveTh + c.EmulsionDet.EPlW

        c.EmulsionDet.BrPackZ = 0.*u.cm
        c.EmulsionDet.BrPackX = 2*0.05*u.cm
        c.EmulsionDet.BrPackY = 2*0.05*u.cm
        
        c.EmulsionDet.BrX = c.EmulsionDet.EmX + c.EmulsionDet.BrPackX
        c.EmulsionDet.BrY = c.EmulsionDet.EmY + c.EmulsionDet.BrPackY
        c.EmulsionDet.BrZ = c.EmulsionDet.n_plates * c.EmulsionDet.AllPW + c.EmulsionDet.EPlW + c.EmulsionDet.BrPackZ

        c.EmulsionDet.xdim = 42.2 *u.cm #external wall dimensions
        c.EmulsionDet.ydim = 42.2 *u.cm
        c.EmulsionDet.TotalWallZDim = 10.0 *u.cm #wall dimension along z, including border
        c.EmulsionDet.WallXDim = 38.6 *u.cm #internal wall dimensions
        c.EmulsionDet.WallYDim = 38.6 *u.cm
        c.EmulsionDet.WallZDim = 8.15 *u.cm
        c.EmulsionDet.WallZBorder_offset = 4.75 * u.mm
        c.EmulsionDet.TTz = 3.0*u.cm
        c.EmulsionDet.zdim = c.EmulsionDet.wall* c.EmulsionDet.TotalWallZDim + c.EmulsionDet.wall*c.EmulsionDet.TTz
        c.EmulsionDet.ShiftX = -8.0*u.cm - c.EmulsionDet.xdim/2.
        c.EmulsionDet.ShiftY = 15.5*u.cm + c.EmulsionDet.ydim/2.

        c.EmulsionDet.startpos =  -25.4750 * u.cm + c.EmulsionDet.z
        c.EmulsionDet.zC = c.EmulsionDet.startpos + c.EmulsionDet.zdim/2.
        
        # survey points in survey coordinate system!
        c.EmulsionDet.Xpos0,c.EmulsionDet.Ypos0,c.EmulsionDet.Zpos0 = 53.5*u.mm,2889.2*u.mm,172.0*u.mm
        c.EmulsionDet.Xpos1,c.EmulsionDet.Ypos1,c.EmulsionDet.Zpos1 = 53.4*u.mm,3019.3*u.mm,172.1*u.mm
        c.EmulsionDet.Xpos2,c.EmulsionDet.Ypos2,c.EmulsionDet.Zpos2 = 53.3*u.mm,3149.2*u.mm,172.3*u.mm
        c.EmulsionDet.Xpos3,c.EmulsionDet.Ypos3,c.EmulsionDet.Zpos3 = 53.3*u.mm,3279.2*u.mm,172.4*u.mm
        c.EmulsionDet.Xpos4,c.EmulsionDet.Ypos4,c.EmulsionDet.Zpos4 = 53.2*u.mm,3409.2*u.mm,172.5*u.mm

        #SciFi parameters
        c.Scifi = AttrDict(z=0*u.cm)
        c.Scifi.xdim = 39.0 * u.cm #sensitive only
        c.Scifi.ydim = 39.0 * u.cm 
        c.Scifi.zdim = c.EmulsionDet.TTz
        c.Scifi.DZ = c.EmulsionDet.BrZ
        c.Scifi.nmats = 3
        c.Scifi.nscifi   = 5

        #SiPM parameters
        c.Scifi.channel_width     = 0.25 *u.mm
        c.Scifi.channel_height     = 1.62 *u.mm
        c.Scifi.sipm_edge = 0.17*u.mm    # on both ends
        c.Scifi.charr_gap = 0.2 *u.mm
        c.Scifi.charr_width = 64 * c.Scifi.channel_width
        c.Scifi.sipm_diegap     = 0.06*u.mm
        c.Scifi.SiPMarray_width = c.Scifi.sipm_edge+c.Scifi.charr_width+c.Scifi.charr_gap+c.Scifi.charr_width+c.Scifi.sipm_edge
        c.Scifi.nsipm_channels  = 128
        c.Scifi.nsipm_mat             = 4
        c.Scifi.nsipms = c.Scifi.nsipm_mat *  c.Scifi.nmats  # 12 per SciFi plane
        c.Scifi.sipmarr_width  = c.Scifi.charr_gap + 2.*c.Scifi.charr_width
        c.Scifi.firstChannelX = -19.528*u.cm  # to agree with SiPM positions of engineering design

        c.Scifi.nfibers_shortrow = 471
        c.Scifi.nfibers_longrow  = 472
        c.Scifi.nfibers_z = 6
        
# Guido: Fibre mat total width 500um less wide than 4 SiPM arrays mounted on a single bar, therefore 32.6mm x 4 -0.5=129.9mm 
        c.Scifi.scifimat_width = (c.Scifi.SiPMarray_width+c.Scifi.sipm_diegap)*c.Scifi.nsipm_mat -0.5*u.mm
        c.Scifi.scifimat_length = c.Scifi.ydim
        c.Scifi.scifimat_z = 0.135 *u.cm
        c.Scifi.epoxymat_z = 0.145 *u.cm
        c.Scifi.scifimat_gap = 0.05 *u.cm
        
        c.Scifi.fiber_length = c.Scifi.scifimat_length
        c.Scifi.scintcore_rmax = 0.011 *u.cm
        c.Scifi.clad1_rmin = c.Scifi.scintcore_rmax
        c.Scifi.clad1_rmax = 0.01175 *u.cm
        c.Scifi.clad2_rmin = c.Scifi.clad1_rmax
        c.Scifi.clad2_rmax = 0.0125 *u.cm

        c.Scifi.horizontal_pitch = 0.0275 *u.cm
        c.Scifi.vertical_pitch = 0.022 *u.cm
        c.Scifi.rowlong_offset = 0.035 *u.cm 
        c.Scifi.rowshort_offset = 0.0215 *u.cm 

        c.Scifi.carbonfiber_z = 0.02 *u.cm
        c.Scifi.honeycomb_z = 0.51 *u.cm
        c.Scifi.glue_z = 0.005 *u.cm
        c.Scifi.airgap_z = 0.04 *u.cm
       
        c.Scifi.plastbar_x = 1.5 *u.cm
        c.Scifi.plastbar_y = c.Scifi.ydim
        c.Scifi.plastbar_z = 0.195 *u.cm

        c.Scifi.scifi_separation = c.Scifi.zdim + c.EmulsionDet.BrZ  
        c.Scifi.offset_z = - c.EmulsionDet.zdim/2 + c.EmulsionDet.BrZ  #SciFi starts at the end of the first ECC
       
        c.Scifi.timeResol = 150.*u.picosecond

        # absolute edge point positions in survey coordinate system
        c.Scifi.Xpos0,c.Scifi.Ypos0,c.Scifi.Zpos0 = 39.3*u.mm,2989.7*u.mm,158.2*u.mm
        c.Scifi.Xpos1,c.Scifi.Ypos1,c.Scifi.Zpos1 = 39.2*u.mm,3119.7*u.mm,158.4*u.mm
        c.Scifi.Xpos2,c.Scifi.Ypos2,c.Scifi.Zpos2 = 39.1*u.mm,3249.7*u.mm,158.5*u.mm
        c.Scifi.Xpos3,c.Scifi.Ypos3,c.Scifi.Zpos3 = 39.1*u.mm,3379.7*u.mm,158.6*u.mm
        c.Scifi.Xpos4,c.Scifi.Ypos4,c.Scifi.Zpos4 = 39.0*u.mm,3509.7*u.mm,158.8*u.mm
# Scifi technical drawing, distance from first channel on vertical / horizontal plane to edge point  225,225,0, xy plane z perpendicularc.Scifi.ydim
        c.Scifi.EdgeAX, c.Scifi.EdgeAY, c.Scifi.EdgeAZ =  225*u.mm, 225*u.mm, 0*u.mm

        c.Scifi.FirstChannelVX,c.Scifi.FirstChannelVY,c.Scifi.FirstChannelVZ = -195.28*u.mm, -200.0*u.mm, -12.92*u.mm

        c.Scifi.FirstChannelHX,c.Scifi.FirstChannelHY,c.Scifi.FirstChannelHZ = -200.0*u.mm, -195.28*u.mm, -7.07*u.mm

# Local Scifi position of first channel on a vertical scifi plane in software
        c.Scifi.LfirstChannelVX,c.Scifi.LfirstChannelVY,c.Scifi.LfirstChannelVZ = -195.135*u.mm, 195.0*u.mm, 11.85*u.mm   # sipm on top

# Local Scifi position of last channel (=first channel on technical drawing) on a horizontal scifi plane in software
        c.Scifi.LfirstChannelHX,c.Scifi.LfirstChannelHY,c.Scifi.LfirstChannelHZ = -195.0*u.mm, 195.178*u.mm,  6.25*u.mm  # sipm on the right side direction away from IP1

# relative mat alignments
        c.Scifi.LocM100,c.Scifi.LocM101,c.Scifi.LocM102 = 0.,0.,0.
        c.Scifi.LocM110,c.Scifi.LocM111,c.Scifi.LocM112 = 0.,0.,0.
        c.Scifi.LocM200,c.Scifi.LocM201,c.Scifi.LocM202 = 0.,0.,0.
        c.Scifi.LocM210,c.Scifi.LocM211,c.Scifi.LocM212 = 0.,0.,0.
        c.Scifi.LocM300,c.Scifi.LocM301,c.Scifi.LocM302 = 0.,0.,0.
        c.Scifi.LocM310,c.Scifi.LocM311,c.Scifi.LocM312 = 0.,0.,0.
        c.Scifi.LocM400,c.Scifi.LocM401,c.Scifi.LocM402 = 0.,0.,0.
        c.Scifi.LocM410,c.Scifi.LocM411,c.Scifi.LocM412 = 0.,0.,0.
        c.Scifi.LocM500,c.Scifi.LocM501,c.Scifi.LocM502 = 0.,0.,0.
        c.Scifi.LocM510,c.Scifi.LocM511,c.Scifi.LocM512 = 0.,0.,0.
# station rotations
        c.Scifi.RotPhiS1,c.Scifi.RotPsiS1,c.Scifi.RotThetaS1 = 0.,0.,0.
        c.Scifi.RotPhiS2,c.Scifi.RotPsiS2,c.Scifi.RotThetaS2 = 0.,0.,0.
        c.Scifi.RotPhiS3,c.Scifi.RotPsiS3,c.Scifi.RotThetaS3 = 0.,0.,0.
        c.Scifi.RotPhiS4,c.Scifi.RotPsiS4,c.Scifi.RotThetaS4 = 0.,0.,0.
        c.Scifi.RotPhiS5,c.Scifi.RotPsiS5,c.Scifi.RotThetaS5 = 0.,0.,0.

# Time alignment Scifi, T0 = station 0,  mat 0 
        c.Scifi.signalSpeed = 15 * u.cm/u.ns
        c.Scifi.station1t,c.Scifi.station1H0t,c.Scifi.station1H1t,c.Scifi.station1H2t,c.Scifi.station1V0t,c.Scifi.station1V1t,c.Scifi.station1V2t  = 0.,0.,0.,0.,0.,0.,0.
        c.Scifi.station2t,c.Scifi.station2H0t,c.Scifi.station2H1t,c.Scifi.station2H2t,c.Scifi.station2V0t,c.Scifi.station2V1t,c.Scifi.station2V2t  = 0.,0.,0.,0.,0.,0.,0.
        c.Scifi.station3t,c.Scifi.station3H0t,c.Scifi.station3H1t,c.Scifi.station3H2t,c.Scifi.station3V0t,c.Scifi.station3V1t,c.Scifi.station3V2t  = 0.,0.,0.,0.,0.,0.,0.
        c.Scifi.station4t,c.Scifi.station4H0t,c.Scifi.station4H1t,c.Scifi.station4H2t,c.Scifi.station4V0t,c.Scifi.station4V1t,c.Scifi.station4V2t  = 0.,0.,0.,0.,0.,0.,0.
        c.Scifi.station5t,c.Scifi.station5H0t,c.Scifi.station5H1t,c.Scifi.station5H2t,c.Scifi.station5V0t,c.Scifi.station5V1t,c.Scifi.station5V2t  = 0.,0.,0.,0.,0.,0.,0.

        c.MuFilter = AttrDict(z=0*u.cm)
        
        #coordinates in local gravity based system
        if year >= 2024:
          c.MuFilter.Veto1Dx,c.MuFilter.Veto1Dy,c.MuFilter.Veto1Dz = 49.3*u.mm, 2761.1*u.mm, 168.1*u.mm
          c.MuFilter.Veto2Dx,c.MuFilter.Veto2Dy,c.MuFilter.Veto2Dz = 49.3*u.mm, 2802.1*u.mm, 148.1*u.mm
          c.MuFilter.Veto3Dx,c.MuFilter.Veto3Dy,c.MuFilter.Veto3Dz = 61.8*u.mm, 2863.1*u.mm, 146.6*u.mm
        else:
          c.MuFilter.Veto1Dx,c.MuFilter.Veto1Dy,c.MuFilter.Veto1Dz = 40.8*u.mm, 2798.3*u.mm, 192.1*u.mm
          c.MuFilter.Veto2Dx,c.MuFilter.Veto2Dy,c.MuFilter.Veto2Dz = 40.6*u.mm, 2839.3*u.mm, 172.1*u.mm

        c.MuFilter.Iron1Dx, c.MuFilter.Iron1Dy, c.MuFilter.Iron1Dz = -22.1*u.mm, 3579.6*u.mm, 146.6*u.mm   
        # US1
        c.MuFilter.Muon1Dx,c.MuFilter.Muon1Dy,c.MuFilter.Muon1Dz = -46.6*u.mm, 3760.2*u.mm, 128.6 *u.mm  
        c.MuFilter.Iron2Dx,  c.MuFilter.Iron2Dy,   c.MuFilter.Iron2Dz   = -22.1*u.mm, 3804.6*u.mm, 136.6*u.mm   
        # US2
        c.MuFilter.Muon2Dx,c.MuFilter.Muon2Dy,c.MuFilter.Muon2Dz = -45.7*u.mm, 3984.1*u.mm, 127.6 *u.mm  
        c.MuFilter.Iron3Dx,  c.MuFilter.Iron3Dy,   c.MuFilter.Iron3Dz    = -22.1*u.mm, 4029.6*u.mm, 126.6*u.mm   
        # US3
        c.MuFilter.Muon3Dx,c.MuFilter.Muon3Dy,c.MuFilter.Muon3Dz = -44.6*u.mm, 4209.5*u.mm, 128.0 *u.mm  
        c.MuFilter.Iron4Dx,  c.MuFilter.Iron4Dy,   c.MuFilter.Iron4Dz    = -22.1*u.mm, 4254.6*u.mm, 116.7*u.mm   
        # US4
        c.MuFilter.Muon4Dx,c.MuFilter.Muon4Dy,c.MuFilter.Muon4Dz = -45.1*u.mm, 4435.6*u.mm, 128.6 *u.mm  
        c.MuFilter.Iron5Dx,   c.MuFilter.Iron5Dy,  c.MuFilter.Iron5Dz    = -22.1*u.mm, 4479.6*u.mm, 127.7*u.mm   
        # US5
        c.MuFilter.Muon5Dx,c.MuFilter.Muon5Dy,c.MuFilter.Muon5Dz = -46.8*u.mm, 4663.0*u.mm, 129.9 *u.mm 
        c.MuFilter.Iron6Dx,   c.MuFilter.Iron6Dy,  c.MuFilter.Iron6Dz    = -22.1*u.mm, 4704.6*u.mm, 127.7*u.mm   
        # DS1
        c.MuFilter.Muon6Dx,c.MuFilter.Muon6Dy,c.MuFilter.Muon6Dz = -45.1*u.mm, 4889.6*u.mm - 1*u.mm, 129.8 *u.mm 
        c.MuFilter.Iron7Dx,   c.MuFilter.Iron7Dy,  c.MuFilter.Iron7Dz    = -22.1*u.mm, 4943.6*u.mm, 127.7*u.mm   
        # DS2
        c.MuFilter.Muon7Dx,c.MuFilter.Muon7Dy,c.MuFilter.Muon7Dz = -45.2*u.mm, 5125.9*u.mm, 132.8 *u.mm 
        c.MuFilter.Iron8Dx,   c.MuFilter.Iron8Dy,  c.MuFilter.Iron8Dz    = -22.1*u.mm, 5183.6*u.mm, 127.7*u.mm   
        # DS3
        c.MuFilter.Muon8Dx,c.MuFilter.Muon8Dy,c.MuFilter.Muon8Dz = -7.9*u.mm, 5396.7*u.mm,  132.5 *u.mm

        c.MuFilter.DS4ZGap = 8.82*u.cm
        # DS4V
        c.MuFilter.Muon9Dx,c.MuFilter.Muon9Dy,c.MuFilter.Muon9Dz =  c.MuFilter.Muon8Dx,     c.MuFilter.Muon8Dy + c.MuFilter.DS4ZGap, c.MuFilter.Muon8Dz
        c.MuFilter.Iron9Dx,  c.MuFilter.Iron9Dy,   c.MuFilter.Iron9Dz    = 177.9*u.mm, 5529.7*u.mm + 1*u.cm,  127.7*u.mm    # move downstream by 1cm to avoid overlap

        # relation between edge and bottom bar for VETO
        c.MuFilter.VETOLocX,c.MuFilter.VETOLocY,c.MuFilter.VETOLocZ = 20.0*u.mm,20.0*u.mm,46.7*u.mm

        # relation between edge and bottom bar for US and DS
        c.MuFilter.DSHLocX,c.MuFilter.DSHLocY,c.MuFilter.DSHLocZ      = 10.5*u.mm, 32.0*u.mm, 11.1*u.mm
        # relation between edge and right bar
        c.MuFilter.DSVLocX,c.MuFilter.DSVLocY,c.MuFilter.DSVLocZ       = 623.0*u.mm, 47.0*u.mm, 641.3*u.mm

        # offsets in Z of first US bar 
        c.MuFilter.USOffZ1 = 4.35*u.mm
        c.MuFilter.USOffZ2 = 5.0*u.mm
        c.MuFilter.USOffZ3 = 4.2*u.mm
        c.MuFilter.USOffZ4 = 5.21*u.mm
        c.MuFilter.USOffZ5 = 4.74*u.mm

        #Veto station parameters
        c.MuFilter.VetonSiPMs = 8
        c.MuFilter.VetonSides  = 2
        if year >= 2024: c.MuFilter.NVetoPlanes = 3
        else: c.MuFilter.NVetoPlanes = 2
        c.MuFilter.NVetoBars    = 7

        c.MuFilter.VetoBarX,c.MuFilter.VetoBarY,c.MuFilter.VetoBarZ = 42 *u.cm, 6 * u.cm, 1 * u.cm
        if year >= 2024: c.MuFilter.Veto3BarX,c.MuFilter.Veto3BarY,c.MuFilter.Veto3BarZ = 5.94*u.cm, 46*u.cm, 1*u.cm
        c.MuFilter.VetoBarGap = 2*30*u.um  # wrapping material

        c.MuFilter.FeX,c.MuFilter.FeY,c.MuFilter.FeZ                  = 80*u.cm, 60*u.cm, 20*u.cm
        c.MuFilter.FeEndX,c.MuFilter.FeEndY,c.MuFilter.FeEndZ = 40*u.cm, 40*u.cm, 20*u.cm
        c.MuFilter.FeBotX,c.MuFilter.FeBotY,c.MuFilter.FeBotZ   = 80*u.cm,   9*u.cm, 40*u.cm

        c.MuFilter.UpstreamDetZ = 2.6*u.cm
        c.MuFilter.UpstreamnSiPMs = 8
        c.MuFilter.UpstreamnSides = 2
        c.MuFilter.NUpstreamPlanes = 5
        c.MuFilter.DownstreamDetZ = 3.9*u.cm
        c.MuFilter.DownstreamnSiPMs = 1
        c.MuFilter.DownstreamnSides = 2   # only for horizontal, vertical only one side
        c.MuFilter.NDownstreamPlanes = 4
        #upstream bars configuration
        c.MuFilter.NUpstreamBars = 10
        c.MuFilter.UpstreamBarX,c.MuFilter.UpstreamBarY,c.MuFilter.UpstreamBarZ = 82.525*u.cm, 6.0*u.cm, 1.0*u.cm
        c.MuFilter.UpstreamBarGap = 0.1*u.mm

        #downstream bars configuration
        c.MuFilter.NDownstreamBars = 60 #n.d.r. both for x and y in this case
        c.MuFilter.DownstreamBarX,c.MuFilter.DownstreamBarY,c.MuFilter.DownstreamBarZ = 82.525*u.cm, 1*u.cm, 1*u.cm
        c.MuFilter.DownstreamBarGap = 0.1*u.mm
        c.MuFilter.DownstreamBarX_ver,c.MuFilter.DownstreamBarY_ver,c.MuFilter.DownstreamBarZ_ver = 1*u.cm, 63.525*u.cm, 1*u.cm

        # DS and US support box, inner, Z pointing upward
        c.MuFilter.SupportBoxD  = 0.5*u.mm  # empty space between bars and box
        c.MuFilter.SupportBoxW = 2*u.mm
        c.MuFilter.DSBoxX1        = c.MuFilter.DSHLocX - c.MuFilter.SupportBoxD
        c.MuFilter.DSBoxX2        = c.MuFilter.DSHLocX + c.MuFilter.DownstreamBarX + c.MuFilter.SupportBoxD
        c.MuFilter.DSBoxZ1        = c.MuFilter.DSHLocZ - c.MuFilter.DownstreamBarY/2 - c.MuFilter.SupportBoxD
        c.MuFilter.DSBoxZ2        = c.MuFilter.DSVLocZ + c.MuFilter.SupportBoxD
        c.MuFilter.DSBoxY1        = c.MuFilter.DSHLocY - c.MuFilter.DownstreamBarZ/2 - c.MuFilter.SupportBoxD
        c.MuFilter.DSBoxY2        = c.MuFilter.DSVLocY + c.MuFilter.DownstreamBarZ/2 + c.MuFilter.SupportBoxD

        c.MuFilter.USBoxY1        = c.MuFilter.DSHLocY - c.MuFilter.DownstreamBarZ/2 - c.MuFilter.SupportBoxD
        c.MuFilter.USBoxY2        = c.MuFilter.DSHLocY + c.MuFilter.DownstreamBarZ/2 + c.MuFilter.SupportBoxD

       # VETO support box 
        c.MuFilter.SupportBoxVW = 4*u.mm
        c.MuFilter.SupportBoxVDH = 0*u.mm

        c.MuFilter.VETOBoxX1        = c.MuFilter.VETOLocX - c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxX2        = c.MuFilter.VETOLocX + c.MuFilter.VetoBarX + c.MuFilter.SupportBoxD

        c.MuFilter.VETOBoxZ1        = c.MuFilter.VETOLocZ - c.MuFilter.VetoBarY/2 - c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxZ2        = c.MuFilter.VETOLocZ + (c.MuFilter.NVetoBars-1)*(c.MuFilter.VetoBarY+c.MuFilter.VetoBarGap) + c.MuFilter.VetoBarY/2 + c.MuFilter.SupportBoxD

        c.MuFilter.VETOBoxY1        = c.MuFilter.VETOLocY - c.MuFilter.VetoBarZ/2 - c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxY2        = c.MuFilter.VETOLocY + c.MuFilter.VetoBarZ/2 + c.MuFilter.SupportBoxD

        if year >= 2024:
          c.MuFilter.SupportBoxVB3 = 6*u.mm
          c.MuFilter.SupportBoxVDH  = 2.0*u.mm  # empty space between 3rd veto plane and box (left-right sides in the hor. plane)

          c.MuFilter.VETOBoxX3        = - c.MuFilter.Veto3BarX/2 - c.MuFilter.SupportBoxD - c.MuFilter.SupportBoxVDH
          c.MuFilter.VETOBoxX4        = (c.MuFilter.NVetoBars-1)*(c.MuFilter.Veto3BarX+c.MuFilter.VetoBarGap) + c.MuFilter.Veto3BarX/2 + c.MuFilter.SupportBoxD + c.MuFilter.SupportBoxVDH

          c.MuFilter.VETOBoxZ3        = - c.MuFilter.SupportBoxD 
          c.MuFilter.VETOBoxZ4        = c.MuFilter.Veto3BarY + c.MuFilter.SupportBoxD

          c.MuFilter.VETOBoxY3        = - c.MuFilter.Veto3BarZ/2 - c.MuFilter.SupportBoxD
          c.MuFilter.VETOBoxY4        = c.MuFilter.Veto3BarZ/2 + c.MuFilter.SupportBoxD

       # VETO/US/DS plane alignment
        c.MuFilter.Veto1ShiftY = 0.0
        c.MuFilter.Veto2ShiftY = 0.0
        if year >= 2024:
          c.MuFilter.Veto3ShiftX = 0.0
        c.MuFilter.US1ShiftY = 0.0
        c.MuFilter.US2ShiftY = 0.0
        c.MuFilter.US3ShiftY = 0.0
        c.MuFilter.US4ShiftY = 0.0
        c.MuFilter.US5ShiftY = 0.0
        c.MuFilter.DS1ShiftY = 0.0
        c.MuFilter.DS1ShiftX = 0.0
        c.MuFilter.DS2ShiftY = 0.0
        c.MuFilter.DS2ShiftX = 0.0
        c.MuFilter.DS3ShiftY = 0.0
        c.MuFilter.DS3ShiftX = 0.0
        c.MuFilter.DS4ShiftX = 0.0

       #digitization parameters
        c.MuFilter.DsAttenuationLength   =  350 * u.cm                #  values between 300 cm and 400cm observed for H6 testbeam
        c.MuFilter.DsTAttenuationLength =  700 * u.cm                # top readout with mirror on bottom
        c.MuFilter.VandUpAttenuationLength = 999 * u.cm        # no significante attenuation observed for H6 testbeam
        c.MuFilter.VandUpSiPMcalibrationL    = 25.*1000.       # 1.65 MeV = 41 qcd 
        c.MuFilter.VandUpSiPMcalibrationS    = 25.*1000.
        c.MuFilter.DsSiPMcalibration             = 25.*1000.
        c.MuFilter.timeResol = 150.*u.picosecond
        c.MuFilter.VandUpPropSpeed    = 12.5*u.cm/u.nanosecond
        c.MuFilter.DsPropSpeed        = 14.9*u.cm/u.nanosecond

        c.Floor = AttrDict(z=48000.*u.cm) # to place tunnel in SND_@LHC coordinate system
        c.Floor.DX = 1.0*u.cm 
        c.Floor.DY = -4.5*u.cm #  subtract 4.5cm to avoid overlaps 
        c.Floor.DZ = 0.

        
        if year >= 2024:
          # Pit on the tunnel floor hosting Veto - using survey coord. system
          # Dimensions
          c.Floor.VetoPitXdim, c.Floor.VetoPitYdim, c.Floor.VetoPitZdim = 500*u.mm, 200*u.mm, 86*u.mm
          # Position == Emuslion wall 1 + a correction in Z
          c.Floor.VetoPitX =  c.EmulsionDet.Xpos0
          c.Floor.VetoPitY =  c.EmulsionDet.Ypos0
          c.Floor.VetoPitZ =  c.EmulsionDet.Zpos0 + c.EmulsionDet.WallZBorder_offset

        #COLDBOX configuration
        c.Floor.Acrylic_width = 5.0*u.cm
        c.Floor.BPoly_width = 4.0*u.cm
        c.Floor.CBFrontWall_xdim = 219.*u.cm
        c.Floor.CBFrontWall_ydim = 170.72*u.cm-c.Floor.Acrylic_width
        c.Floor.CBLatWall_zdim = 176.0*u.cm
        c.Floor.CBTiny_zdim = 17.0*u.cm
        c.Floor.CBExtra_zdim = 41.0*u.cm
        c.Floor.CBExtra_xdim = 67.5 *u.cm
        c.Floor.SlopedWall_zproj = 110.0*u.cm
        c.Floor.MFeBlockX = c.MuFilter.FeX
        c.Floor.MFeBlockY = c.MuFilter.FeY
        c.Floor.MFeBlockZ = c.MuFilter.FeZ
