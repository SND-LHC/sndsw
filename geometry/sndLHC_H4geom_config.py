import ROOT as r
import shipunit as u
from ShipGeoConfig import AttrDict, ConfigRegistry

with_tungsten = False
if "tb_2024_mc" in globals():
    tb_2024_mc = True
    if  globals()["target_material"]=="W":
      with_tungsten = True
else: tb_2024_mc = False

with ConfigRegistry.register_config("basic") as c:
# cave parameters
        c.cave = AttrDict(z=0*u.cm)

        # Antonia, 482000mm (FASER+2, P3) + 1017mm (DZ) + 245mm (centre emulsion),z=483262./10.*u.cm
        # centre emulsion now 326.2cm downstream from origin.
        c.EmulsionDet = AttrDict(z=326.2*u.cm)
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
        c.Scifi.epoxymat_z = 0.162 *u.cm
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
        c.Scifi.honeycomb_z = 0.5 *u.cm
       
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

# Time alignment Scifi, T0 = station 0,  mat 0 
        c.Scifi.signalSpeed = 15 * u.cm/u.ns

        c.MuFilter = AttrDict(z=0*u.cm)
        #coordinates in local gravity based system
        c.MuFilter.Veto1Dx,c.MuFilter.Veto1Dy,c.MuFilter.Veto1Dz = 40.8*u.mm, 2798.3*u.mm, 192.1*u.mm
        c.MuFilter.Veto2Dx,c.MuFilter.Veto2Dy,c.MuFilter.Veto2Dz = 40.6*u.mm, 2839.3*u.mm, 172.1*u.mm       
        c.MuFilter.Iron1Dx, c.MuFilter.Iron1Dy, c.MuFilter.Iron1Dz = -22.1*u.mm, 3579.6*u.mm, 146.6*u.mm   
        # US1
        c.MuFilter.Muon1Dx,c.MuFilter.Muon1Dy,c.MuFilter.Muon1Dz = -46.6*u.mm, 3760.2*u.mm, 128.6 *u.mm  
        c.MuFilter.Iron2Dx,  c.MuFilter.Iron2Dy,   c.MuFilter.Iron2Dz   = -22.1*u.mm, 3804.6*u.mm, 136.6*u.mm   
        # US2
        c.MuFilter.Muon2Dx,c.MuFilter.Muon2Dy,c.MuFilter.Muon2Dz = -45.7*u.mm, 3984.1*u.mm, 127.6 *u.mm  
        c.MuFilter.Iron3Dx,  c.MuFilter.Iron3Dy,   c.MuFilter.Iron3Dz    = -22.1*u.mm, 4029.6*u.mm, 1318.6*u.mm   
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
        c.MuFilter.NVetoPlanes = 2
        c.MuFilter.NVetoBars    = 7

        c.MuFilter.VetoBarX,c.MuFilter.VetoBarY,c.MuFilter.VetoBarZ = 42 *u.cm, 6 * u.cm, 1 * u.cm
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
        c.MuFilter.VETOBoxX1        = c.MuFilter.VETOLocX - c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxX2        = c.MuFilter.VETOLocX + c.MuFilter.VetoBarX + c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxZ1        = c.MuFilter.VETOLocZ - c.MuFilter.VetoBarY/2 - c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxZ2        = c.MuFilter.VETOLocZ + (c.MuFilter.NVetoBars-1)*(c.MuFilter.VetoBarY+c.MuFilter.VetoBarGap) + c.MuFilter.VetoBarY/2 + c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxY1        = c.MuFilter.VETOLocY - c.MuFilter.VetoBarZ/2 - c.MuFilter.SupportBoxD
        c.MuFilter.VETOBoxY2        = c.MuFilter.VETOLocY + c.MuFilter.VetoBarZ/2 + c.MuFilter.SupportBoxD

       # VETO/US/DS plane alignment
        c.MuFilter.Veto1ShiftY =  0.11 * u.cm
        c.MuFilter.Veto2ShiftY =  -0.04 * u.cm
        c.MuFilter.US1ShiftY =   0.10 * u.cm
        c.MuFilter.US2ShiftY =   0.26 * u.cm
        c.MuFilter.US3ShiftY =   0.24 * u.cm
        c.MuFilter.US4ShiftY =   0.31 * u.cm
        c.MuFilter.US5ShiftY =   0.34 * u.cm
        c.MuFilter.DS1ShiftY =   0.43 * u.cm
        c.MuFilter.DS1ShiftX =    1.13 * u.cm
        c.MuFilter.DS2ShiftY =   0.53 * u.cm
        c.MuFilter.DS2ShiftX =    1.31 * u.cm
        c.MuFilter.DS3ShiftY =   0.61 * u.cm
        c.MuFilter.DS3ShiftX =    1.35 * u.cm
        c.MuFilter.DS4ShiftX =    1.39 * u.cm
        
       #digitization parameters
        c.MuFilter.DsAttenuationLength   =  350 * u.cm                #  values between 300 cm and 400cm observed for H6 testbeam
        c.MuFilter.DsTAttenuationLength =  700 * u.cm                # top readout with mirror on bottom
        c.MuFilter.VandUpAttenuationLength = 999 * u.cm        # no significante attenuation observed for H6 testbeam
        c.MuFilter.VandUpSiPMcalibrationL    = 25.*1000.       # 1.65 MeV = 41 qcd 
        c.MuFilter.VandUpSiPMcalibrationS    = 25.*1000.
        c.MuFilter.DsSiPMcalibration             = 25.*1000.
        c.MuFilter.timeResol = 150.*u.picosecond
        c.MuFilter.VandUpPropSpeed    = 12.5*u.cm/u.nanosecond
        c.MuFilter.DsPropSpeed        = 14.3*u.cm/u.nanosecond

        c.Floor = AttrDict(z=48000.*u.cm) # to place tunnel in SND_@LHC coordinate system
        c.Floor.DX = 1.0*u.cm 
        c.Floor.DY = -4.5*u.cm #  subtract 4.5cm to avoid overlaps 
        c.Floor.DZ = 0.

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
        
# for H4 / H6 / H8 / HX testbeam and commissioning
        H4 = True
        if H4:
           c.Floor.z = 0   # no tunnel, no slope

           # no Veto
           c.MuFilter.NVetoPlanes = 0
           # no US
           c.MuFilter.NUpstreamPlanes = 0
           
           # only 1 DS station
           c.MuFilter.NDownstreamPlanes = 1
           # remove most downstream iron block and its bottom part
           c.MuFilter.FeEndX,c.MuFilter.FeEndY,c.MuFilter.FeEndZ = 0*u.cm, 0*u.cm, 0*u.cm
           c.MuFilter.FeBotX,c.MuFilter.FeBotY,c.MuFilter.FeBotZ   = 0*u.cm, 0*u.cm, 0*u.cm
           # remove the iron block before the DS1
           c.MuFilter.FeX,c.MuFilter.FeY,c.MuFilter.FeZ = 0*u.cm, 0*u.cm, 0*u.cm

           # removing the emulsion+W target for the second part of the TB,
           # where we have Fe blocks btw SciFi stations
           c.EmulsionDet.wall = 0
           if with_tungsten:
             c.EmulsionDet = AttrDict(z=0*u.cm)
             c.EmulsionDet.PassiveOption = 1 #0 makes emulsion volumes active, 1 makes all emulsion volumes passive
             c.EmulsionDet.row = 2
             c.EmulsionDet.col = 2
             c.EmulsionDet.wall= 2
             c.EmulsionDet.target = 0 #number of neutrino target volumes

             c.EmulsionDet.xdim = 42.2 *u.cm #external wall dimensions
             c.EmulsionDet.ydim = 42.2 *u.cm
             c.EmulsionDet.TotalWallZDim = 10.0 *u.cm #wall dimension along z, including border
             c.EmulsionDet.WallXDim = 38.6 *u.cm #internal wall dimensions
             c.EmulsionDet.WallYDim = 38.6 *u.cm
             c.EmulsionDet.WallZDim = 8.15 *u.cm
             c.EmulsionDet.WallZBorder_offset = 4.75 * u.mm
             c.EmulsionDet.TTz = 4.0*u.cm # thickness of target station!
             c.EmulsionDet.zdim = c.EmulsionDet.wall* c.EmulsionDet.TotalWallZDim + c.EmulsionDet.wall*c.EmulsionDet.TTz

             c.EmulsionDet.n_plates = 59 # the most downstream 28 layers are W+plastic
             c.EmulsionDet.n_tungsten_plates_tb24 = 28
             c.EmulsionDet.EmX = 19.2 * u.cm
             c.EmulsionDet.EmY = 19.2 * u.cm
             c.EmulsionDet.EmTh = 0.0 * u.cm # no emulsion 
             c.EmulsionDet.PBTh = 0.0175 * u.cm + 2*0.0070 * u.cm # this is the plastic base tickness -same value as the full em+plastic basein TI18
             c.EmulsionDet.PassiveTh = 0.1 * u.cm # this is the tungsten
             c.EmulsionDet.EPlW = c.EmulsionDet.PBTh
             c.EmulsionDet.AllPW = c.EmulsionDet.PassiveTh + c.EmulsionDet.EPlW # this is used to add layers of W+emulsion

             c.EmulsionDet.BrPackZ = 0.*u.cm
             c.EmulsionDet.BrPackX = 2*0.05*u.cm
             c.EmulsionDet.BrPackY = 2*0.05*u.cm
           
             c.EmulsionDet.BrX = c.EmulsionDet.EmX + c.EmulsionDet.BrPackX
             c.EmulsionDet.BrY = c.EmulsionDet.EmY + c.EmulsionDet.BrPackY
             c.EmulsionDet.BrZ = c.EmulsionDet.n_plates * c.EmulsionDet.AllPW + c.EmulsionDet.EPlW + c.EmulsionDet.BrPackZ

             # do we use that? #FIXME
             c.EmulsionDet.ShiftX = 0.#-8.0*u.cm - c.EmulsionDet.xdim/2.
             c.EmulsionDet.ShiftY = 0.#15.5*u.cm + c.EmulsionDet.ydim/2.
             # do we use that? #FIXME
             c.EmulsionDet.startpos = 0.#-25.4750 * u.cm + c.EmulsionDet.z
             c.EmulsionDet.zC = 0.# c.EmulsionDet.startpos + c.EmulsionDet.zdim/2.

             # survey points in survey coordinate system!
             c.EmulsionDet.Xpos0,c.EmulsionDet.Ypos0,c.EmulsionDet.Zpos0 = 318.6*u.mm,3218.8*u.mm,380.2*u.mm
             c.EmulsionDet.Xpos1,c.EmulsionDet.Ypos1,c.EmulsionDet.Zpos1 = 318.6*u.mm,3383.8*u.mm,380.2*u.mm


           # set SciFi modules
           c.Scifi.xdim = 13.0*u.cm #sensitive only
           c.Scifi.ydim = 13.0*u.cm
           c.Scifi.zdim = 4.0*u.cm # z thickness of Scifi station, must match c.EmulsionDet.TTz
           c.Scifi.nmats = 1
           c.Scifi.nscifi = 4
           c.Scifi.nfibers_z = 7
           c.Scifi.scifimat_z = 0.16*u.cm # thickness of a mat of 7 fiber layers
           c.Scifi.scifimat_length  = c.Scifi.ydim
           c.Scifi.fiber_length = c.Scifi.scifimat_length
           c.Scifi.plane_gap = 6*u.mm # an air gap btw X an Y planes, in TI18 case this is controlled via c.Scifi.plastbar_z
           c.Scifi.tedlar_to_plane = 5.42*u.mm # an air gap btw protective tedlar sheet and a sensitive plane
           #c.Scifi.tedlar_z = 50*u.um # not included in the sw detector model
           # offset between the edge of a baby module frame and the upstream tedlar inside it
           c.Scifi.frame_offset = 0.8*u.cm
           # offsets between the upstream tedlar sheet and the upstream passive block
           c.Scifi.station_offset1 = c.Scifi.frame_offset+1.4*u.cm
           c.Scifi.station_offset2 = c.Scifi.frame_offset+1.4*u.cm
           c.Scifi.station_offset3 = c.Scifi.frame_offset+1.4*u.cm
           c.Scifi.channelTimeAlignment = 1

           if not with_tungsten:
             # add variable size blocks upstream of SciFi stations
             # complete removal of a wall goes with commenting the respective line below
             #c.Scifi.FeTargetX3, c.Scifi.FeTargetY3, c.Scifi.FeTargetZ3  = 30.*u.cm, 10.*u.cm, 30.*u.cm
             c.Scifi.FeTargetX4, c.Scifi.FeTargetY4, c.Scifi.FeTargetZ4  = 30.*u.cm, 10.*u.cm, 30.*u.cm
             # flag showing whether the passive material (FeBlocks) are to be centerred around the Scifi
             c.Scifi.PassiveBlocknotCenterred = 1

# absolute edge point positions in survey coordinate system (survey is 'by eye' for now)
           c.Scifi.Xpos0,c.Scifi.Ypos0,c.Scifi.Zpos0 = 156.6*u.mm,3186.3*u.mm,218.2*u.mm
           c.Scifi.Xpos1,c.Scifi.Ypos1,c.Scifi.Zpos1 = 156.6*u.mm,3351.3*u.mm,221.7*u.mm # 16.5cm step
           c.Scifi.Xpos2,c.Scifi.Ypos2,c.Scifi.Zpos2 = 156.6*u.mm,3516.3*u.mm,225.1*u.mm # 16.5cm step
           c.Scifi.Xpos3,c.Scifi.Ypos3,c.Scifi.Zpos3 = 156.6*u.mm,3676.3*u.mm,228.5*u.mm # 16cm step

           # DS1 - since there are no US planes, we take the item labelled Muon1!
           # DS spatial alignment is included in the values
           c.MuFilter.Muon1Dx,c.MuFilter.Muon1Dy,c.MuFilter.Muon1Dz = 318.6*u.mm, 4066.3*u.mm, 230.2*u.mm

# from Scifi track alignment
           if tb_2024_mc :
# spatial
             c.Scifi.LocM100, c.Scifi.LocM110 = 0.000*u.um, 0.000*u.um
             c.Scifi.LocM200, c.Scifi.LocM210 = 0.000*u.um, 0.000*u.um
             c.Scifi.LocM300, c.Scifi.LocM310 = 0.000*u.um, 0.000*u.um
             c.Scifi.LocM400, c.Scifi.LocM410 = 0.000*u.um, 0.000*u.um
             c.Scifi.RotPhiS10,c.Scifi.RotPsiS10,c.Scifi.RotThetaS10 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS11,c.Scifi.RotPsiS11,c.Scifi.RotThetaS11 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS20,c.Scifi.RotPsiS20,c.Scifi.RotThetaS20 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS21,c.Scifi.RotPsiS21,c.Scifi.RotThetaS21 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS30,c.Scifi.RotPsiS30,c.Scifi.RotThetaS30 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS31,c.Scifi.RotPsiS31,c.Scifi.RotThetaS31 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS40,c.Scifi.RotPsiS40,c.Scifi.RotThetaS40 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS41,c.Scifi.RotPsiS41,c.Scifi.RotThetaS41 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
# Time alignment Scifi, T0 = station 0,  mat 0 
             c.Scifi.station1t,c.Scifi.station1H0t,c.Scifi.station1V0t  =  0.000*u.ns,  0.000*u.ns,  0.000*u.ns
             c.Scifi.station2t,c.Scifi.station2H0t,c.Scifi.station2V0t  =  0.000*u.ns,  0.000*u.ns,  0.000*u.ns
             c.Scifi.station3t,c.Scifi.station3H0t,c.Scifi.station3V0t  =  0.000*u.ns,  0.000*u.ns,  0.000*u.ns
             c.Scifi.station4t,c.Scifi.station4H0t,c.Scifi.station4V0t  =  0.000*u.ns,  0.000*u.ns,  0.000*u.ns
           else :
# spatial
             c.Scifi.LocM100, c.Scifi.LocM110 = 2800.00*u.um, 420.01*u.um
             c.Scifi.LocM200, c.Scifi.LocM210 = 554.35*u.um, 374.96*u.um
             c.Scifi.LocM300, c.Scifi.LocM310 = -614.41**u.um, 56.48*u.um
             c.Scifi.LocM400, c.Scifi.LocM410 = -154.11*u.um, 472.07*u.um
             c.Scifi.RotPhiS10,c.Scifi.RotPsiS10,c.Scifi.RotThetaS10 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS11,c.Scifi.RotPsiS11,c.Scifi.RotThetaS11 =     0.00*u.mrad,    1.65*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS20,c.Scifi.RotPsiS20,c.Scifi.RotThetaS20 =     0.00*u.mrad,    2.23*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS21,c.Scifi.RotPsiS21,c.Scifi.RotThetaS21 =     0.00*u.mrad,    0.68*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS30,c.Scifi.RotPsiS30,c.Scifi.RotThetaS30 =     0.00*u.mrad,    1.06*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS31,c.Scifi.RotPsiS31,c.Scifi.RotThetaS31 =     0.00*u.mrad,    0.82*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS40,c.Scifi.RotPsiS40,c.Scifi.RotThetaS40 =     0.00*u.mrad,    1.61*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS41,c.Scifi.RotPsiS41,c.Scifi.RotThetaS41 =     0.00*u.mrad,    2.22*u.mrad,    0.00*u.mrad
# Time alignment Scifi, T0 = station 0,  mat 0 
             c.Scifi.station1t,c.Scifi.station1H0t,c.Scifi.station1V0t  =  0.000*u.ns,  0.000*u.ns,  -1.714*u.ns
             c.Scifi.station2t,c.Scifi.station2H0t,c.Scifi.station2V0t  =  -0.510*u.ns,  0.000*u.ns,  0.003*u.ns
             c.Scifi.station3t,c.Scifi.station3H0t,c.Scifi.station3V0t  =  1.447*u.ns,  0.000*u.ns,  0.520*u.ns
             c.Scifi.station4t,c.Scifi.station4H0t,c.Scifi.station4V0t  =  -0.014*u.ns,  0.000*u.ns,  2.357*u.ns
# Additional alignment for station 2H(one mat) perhaps needed due to calibration issue
             c.Scifi.station2000XXXt = [-1.956*u.ns,-2.231*u.ns,-1.250*u.ns,-1.474*u.ns,-2.215*u.ns,-2.027*u.ns,-1.713*u.ns,-1.899*u.ns,
                                        -1.875*u.ns,-1.848*u.ns,-1.862*u.ns,-2.184*u.ns,-1.855*u.ns,-2.066*u.ns,-1.252*u.ns,-2.268*u.ns,
                                        -2.370*u.ns,-1.414*u.ns,-1.577*u.ns,-1.909*u.ns,-1.952*u.ns,-1.491*u.ns,-1.269*u.ns,-1.741*u.ns,
                                        -1.823*u.ns,-2.120*u.ns,-2.224*u.ns,-1.758*u.ns,-1.715*u.ns,-2.037*u.ns,-2.219*u.ns,-2.667*u.ns,
                                        -2.269*u.ns,-1.526*u.ns,-1.840*u.ns,-1.258*u.ns,-2.459*u.ns,-2.631*u.ns,-1.335*u.ns,-1.448*u.ns,
                                        -1.364*u.ns,-1.589*u.ns,-1.647*u.ns,-1.677*u.ns,-1.508*u.ns,-1.548*u.ns,-1.289*u.ns,-1.249*u.ns,
                                        -1.388*u.ns,-1.654*u.ns,-1.945*u.ns,-2.371*u.ns,-2.308*u.ns,-1.310*u.ns,-1.497*u.ns,-1.928*u.ns,
                                        -2.160*u.ns,-1.470*u.ns,-1.452*u.ns,-2.263*u.ns,-2.291*u.ns,-1.557*u.ns,-1.396*u.ns,-1.003*u.ns,
                                        -0.735*u.ns,-1.354*u.ns,-1.508*u.ns,-0.978*u.ns,-1.111*u.ns,-0.750*u.ns,-1.168*u.ns,-1.891*u.ns,
                                        -2.219*u.ns,-0.647*u.ns,-0.557*u.ns,-0.518*u.ns,-0.937*u.ns,-1.397*u.ns,-1.702*u.ns,-1.624*u.ns,
                                        -1.055*u.ns,-1.457*u.ns,-1.114*u.ns,-1.283*u.ns,-1.562*u.ns,-1.997*u.ns,-1.717*u.ns,-0.753*u.ns,
                                        -0.628*u.ns,-1.031*u.ns,-1.405*u.ns,-0.910*u.ns,-0.723*u.ns,-0.214*u.ns,-0.762*u.ns,-1.101*u.ns,
                                        -0.685*u.ns,-0.799*u.ns,-1.127*u.ns,-1.164*u.ns,-1.375*u.ns,-1.535*u.ns,-1.577*u.ns,-1.604*u.ns,
                                        -1.492*u.ns,-1.137*u.ns,-1.454*u.ns,-1.812*u.ns,-1.558*u.ns,-1.551*u.ns,-1.475*u.ns,-1.361*u.ns,
                                        -1.071*u.ns,-1.525*u.ns,-1.471*u.ns,-0.709*u.ns,-1.032*u.ns,-0.837*u.ns,-0.945*u.ns,-0.657*u.ns,
                                        -1.457*u.ns,-1.759*u.ns,-1.442*u.ns,-1.124*u.ns,-1.246*u.ns,-1.320*u.ns,-1.457*u.ns,-0.479*u.ns]
             c.Scifi.station2001XXXt = [-1.387*u.ns,-1.486*u.ns,-1.424*u.ns,-0.859*u.ns,-0.838*u.ns,-1.253*u.ns,-1.724*u.ns,-1.970*u.ns,
                                        -1.731*u.ns,-0.977*u.ns,-0.948*u.ns,-1.277*u.ns,-1.225*u.ns,-1.032*u.ns,-1.747*u.ns,-1.799*u.ns,
                                        -0.858*u.ns,-1.966*u.ns,-2.336*u.ns,-1.940*u.ns,-1.809*u.ns,-1.265*u.ns,-1.688*u.ns,-1.636*u.ns,
                                        -1.602*u.ns,-1.785*u.ns,-1.958*u.ns,-2.064*u.ns,-2.039*u.ns,-1.870*u.ns,-2.055*u.ns,-1.428*u.ns,
                                        -1.206*u.ns,-1.516*u.ns,-1.528*u.ns,-1.546*u.ns,-1.537*u.ns,-1.860*u.ns,-1.399*u.ns,-1.403*u.ns,
                                        -1.581*u.ns,-1.223*u.ns,-1.864*u.ns,-2.190*u.ns,-1.152*u.ns,-1.617*u.ns,-1.435*u.ns,-0.957*u.ns,
                                        -1.370*u.ns,-1.562*u.ns,-1.780*u.ns,-1.456*u.ns,-1.493*u.ns,-1.875*u.ns,-1.713*u.ns,-1.054*u.ns,
                                        -1.241*u.ns,-1.240*u.ns,-0.540*u.ns,-0.872*u.ns,-1.132*u.ns,-1.139*u.ns,-1.130*u.ns,-0.897*u.ns,
                                        -1.188*u.ns,-1.626*u.ns,-1.564*u.ns,-1.865*u.ns,-2.079*u.ns,-1.943*u.ns,-0.764*u.ns,-0.936*u.ns,
                                        -1.313*u.ns,-1.005*u.ns,-1.335*u.ns,-1.594*u.ns,-1.639*u.ns,-1.580*u.ns,-1.701*u.ns,-1.572*u.ns,
                                        -1.927*u.ns,-1.390*u.ns,-2.407*u.ns,-2.865*u.ns,-1.308*u.ns,-1.489*u.ns,-1.921*u.ns,-2.161*u.ns,
                                        -1.243*u.ns,-1.563*u.ns,-1.855*u.ns,-2.163*u.ns,-1.930*u.ns,-2.251*u.ns,-1.584*u.ns,-1.448*u.ns,
                                        -2.016*u.ns,-1.721*u.ns,-1.447*u.ns,-1.220*u.ns,-1.139*u.ns,-1.932*u.ns,-1.753*u.ns,-1.781*u.ns,
                                        -1.751*u.ns,-1.583*u.ns,-1.193*u.ns,-1.516*u.ns,-1.061*u.ns,-1.206*u.ns,-1.236*u.ns,-1.638*u.ns,
                                        -1.777*u.ns,-1.362*u.ns,-1.385*u.ns,-1.465*u.ns,-1.185*u.ns,-0.964*u.ns,-0.869*u.ns,-1.308*u.ns,
                                        -1.806*u.ns,-1.900*u.ns,-1.898*u.ns,-1.365*u.ns,-1.008*u.ns,-1.479*u.ns,-1.907*u.ns,-1.474*u.ns]
             c.Scifi.station2002XXXt = [0.627*u.ns,0.230*u.ns,0.202*u.ns,-0.234*u.ns,-0.176*u.ns,0.004*u.ns,0.302*u.ns,0.697*u.ns,
                                        0.477*u.ns,-0.210*u.ns,-0.406*u.ns,-0.086*u.ns,-0.129*u.ns,-0.625*u.ns,0.104*u.ns,-0.002*u.ns,
                                        -0.683*u.ns,-0.065*u.ns,0.746*u.ns,0.408*u.ns,0.226*u.ns,-0.507*u.ns,0.324*u.ns,0.330*u.ns,
                                        0.244*u.ns,0.155*u.ns,0.552*u.ns,0.442*u.ns,0.462*u.ns,0.113*u.ns,0.477*u.ns,0.086*u.ns,
                                        -0.384*u.ns,0.077*u.ns,-0.004*u.ns,0.095*u.ns,-0.264*u.ns,0.418*u.ns,0.051*u.ns,-0.134*u.ns,
                                        0.239*u.ns,-0.055*u.ns,-0.113*u.ns,0.542*u.ns,-0.251*u.ns,-0.027*u.ns,0.067*u.ns,-0.707*u.ns,
                                        0.018*u.ns,-0.017*u.ns,0.378*u.ns,0.002*u.ns,-0.178*u.ns,0.454*u.ns,0.334*u.ns,-0.379*u.ns,
                                        -0.552*u.ns,-0.050*u.ns,-0.811*u.ns,-0.457*u.ns,-0.107*u.ns,-0.078*u.ns,-0.028*u.ns,0.149*u.ns,
                                        0.348*u.ns,0.301*u.ns,0.234*u.ns,0.373*u.ns,0.573*u.ns,0.047*u.ns,-0.689*u.ns,-0.639*u.ns,
                                        -0.169*u.ns,-0.484*u.ns,-0.015*u.ns,0.096*u.ns,0.154*u.ns,0.057*u.ns,0.078*u.ns,-0.102*u.ns,
                                        0.180*u.ns,-0.130*u.ns,0.378*u.ns,0.407*u.ns,-0.073*u.ns,0.028*u.ns,-0.049*u.ns,0.214*u.ns,
                                        -0.379*u.ns,0.009*u.ns,0.044*u.ns,0.302*u.ns,-0.012*u.ns,0.533*u.ns,-0.158*u.ns,-0.383*u.ns,
                                        0.548*u.ns,0.160*u.ns,-0.058*u.ns,-0.617*u.ns,-0.862*u.ns,0.477*u.ns,0.282*u.ns,0.349*u.ns,
                                        0.364*u.ns,-0.080*u.ns,-0.473*u.ns,-0.022*u.ns,-0.444*u.ns,-0.181*u.ns,-0.260*u.ns,0.240*u.ns,
                                        0.452*u.ns,0.114*u.ns,0.192*u.ns,0.168*u.ns,-0.085*u.ns,-0.228*u.ns,-0.370*u.ns,-0.341*u.ns,
                                        0.516*u.ns,0.463*u.ns,0.323*u.ns,-0.061*u.ns,-0.245*u.ns,-0.205*u.ns,0.531*u.ns,0.772*u.ns]
             c.Scifi.station2003XXXt = [0.800*u.ns,0.586*u.ns,0.113*u.ns,0.188*u.ns,0.760*u.ns,0.724*u.ns,0.409*u.ns,0.613*u.ns,
                                        0.644*u.ns,0.538*u.ns,0.316*u.ns,0.696*u.ns,0.334*u.ns,0.490*u.ns,-0.216*u.ns,-0.351*u.ns,
                                        0.776*u.ns,0.123*u.ns,0.183*u.ns,0.488*u.ns,0.406*u.ns,0.024*u.ns,-0.253*u.ns,0.376*u.ns,
                                        0.319*u.ns,0.505*u.ns,0.764*u.ns,0.352*u.ns,0.220*u.ns,0.437*u.ns,0.404*u.ns,0.831*u.ns,
                                        0.377*u.ns,-0.279*u.ns,-0.166*u.ns,-0.778*u.ns,0.675*u.ns,0.466*u.ns,-0.063*u.ns,0.030*u.ns,
                                        0.040*u.ns,0.165*u.ns,0.257*u.ns,0.241*u.ns,0.219*u.ns,0.165*u.ns,-0.029*u.ns,-0.041*u.ns,
                                        0.163*u.ns,0.187*u.ns,0.488*u.ns,0.878*u.ns,0.561*u.ns,0.010*u.ns,0.133*u.ns,0.344*u.ns,
                                        0.572*u.ns,-0.019*u.ns,-0.081*u.ns,0.634*u.ns,0.706*u.ns,0.339*u.ns,0.252*u.ns,0.203*u.ns,
                                        -0.029*u.ns,0.007*u.ns,-0.018*u.ns,-0.370*u.ns,-0.266*u.ns,-0.621*u.ns,-0.403*u.ns,0.073*u.ns,
                                        0.009*u.ns,-0.691*u.ns,-0.736*u.ns,-0.940*u.ns,-0.656*u.ns,-0.158*u.ns,0.097*u.ns,-0.492*u.ns,
                                        -0.952*u.ns,-0.185*u.ns,-0.356*u.ns,-0.156*u.ns,-0.001*u.ns,0.237*u.ns,-0.179*u.ns,-0.736*u.ns,
                                        -0.879*u.ns,-0.930*u.ns,-0.139*u.ns,-0.575*u.ns,-0.959*u.ns,-1.411*u.ns,-0.991*u.ns,-0.358*u.ns,
                                        -0.799*u.ns,-0.879*u.ns,-0.304*u.ns,-0.227*u.ns,-0.089*u.ns,0.099*u.ns,0.153*u.ns,0.210*u.ns,
                                        0.156*u.ns,-0.186*u.ns,-0.115*u.ns,0.372*u.ns,0.183*u.ns,0.129*u.ns,0.155*u.ns,-0.129*u.ns,
                                        -0.415*u.ns,0.010*u.ns,-0.138*u.ns,-0.592*u.ns,-0.460*u.ns,-0.613*u.ns,-0.628*u.ns,-0.992*u.ns,
                                        0.004*u.ns,0.323*u.ns,-0.042*u.ns,-0.180*u.ns,-0.041*u.ns,-0.095*u.ns,-0.092*u.ns,-0.173*u.ns]
