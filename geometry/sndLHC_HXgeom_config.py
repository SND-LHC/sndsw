import ROOT as r
import shipunit as u
from ShipGeoConfig import AttrDict, ConfigRegistry

if "tb_2023_mc" in globals():
    tb_2023_mc = True
else: tb_2023_mc = False

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
        
# for H6 / H8 / HX testbeam and commissioning
        HX = True
        if HX:
           c.Floor.z = 0   # no tunnel, no slope

           # no Veto
           c.MuFilter.NVetoPlanes = 0
           
           # only 1 DS station
           c.MuFilter.NDownstreamPlanes = 1
           # remove most downstream iron block and its bottom part
           c.MuFilter.FeEndX,c.MuFilter.FeEndY,c.MuFilter.FeEndZ = 0*u.cm, 0*u.cm, 0*u.cm
           c.MuFilter.FeBotX,c.MuFilter.FeBotY,c.MuFilter.FeBotZ   = 0*u.cm, 0*u.cm, 0*u.cm

           # removing the emulsion+W target
           c.EmulsionDet.wall = 0

           # set baby SciFi modules
           # they have much simpler internal structure compared to the TI18 SciFi
           # most notably they have 7 layer of fibers per mat
           c.Scifi.xdim = 13.0*u.cm #sensitive only
           c.Scifi.ydim = 13.0*u.cm
           c.Scifi.zdim = 3*u.cm # maybe not needed
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
           c.Scifi.station_offset1 = c.Scifi.frame_offset
           c.Scifi.station_offset2 = c.Scifi.frame_offset+1.2*u.cm
           c.Scifi.station_offset3 = c.Scifi.frame_offset+0.6*u.cm

           # add 3 variable size iron blocks upstream of SciFi stations
           # complete removal of a wall goes with commenting the respective line below
           c.Scifi.FeTargetX2, c.Scifi.FeTargetY2, c.Scifi.FeTargetZ2  = 30.*u.cm, 10.*u.cm, 30.*u.cm
           c.Scifi.FeTargetX3, c.Scifi.FeTargetY3, c.Scifi.FeTargetZ3  = 30.*u.cm, 10.*u.cm, 30.*u.cm
           c.Scifi.FeTargetX4, c.Scifi.FeTargetY4, c.Scifi.FeTargetZ4  = 30.*u.cm, 10.*u.cm, 30.*u.cm

# absolute edge point positions in survey coordinate system (survey is 'by eye' for now)
           c.Scifi.Xpos0,c.Scifi.Ypos0,c.Scifi.Zpos0 = 156.6*u.mm,3136.48*u.mm,218.2*u.mm
           c.Scifi.Xpos1,c.Scifi.Ypos1,c.Scifi.Zpos1 = 156.6*u.mm,3264.52*u.mm,218.4*u.mm
           c.Scifi.Xpos2,c.Scifi.Ypos2,c.Scifi.Zpos2 = 156.6*u.mm,3404.56*u.mm,218.5*u.mm
           c.Scifi.Xpos3,c.Scifi.Ypos3,c.Scifi.Zpos3 = 156.6*u.mm,3538.6*u.mm,218.6*u.mm
# and MuFilter - survey measurements
           c.MuFilter.Iron1Dx, c.MuFilter.Iron1Dy, c.MuFilter.Iron1Dz = -22.1*u.mm, 3559.6*u.mm, 128.6*u.mm
           # US1
           c.MuFilter.Muon1Dx,c.MuFilter.Muon1Dy,c.MuFilter.Muon1Dz = -46.6*u.mm, 3740.2*u.mm, 128.6 *u.mm
           c.MuFilter.Iron2Dx,  c.MuFilter.Iron2Dy,   c.MuFilter.Iron2Dz   = -22.1*u.mm, 3784.6*u.mm, 128.6*u.mm
           # US2
           c.MuFilter.Muon2Dx,c.MuFilter.Muon2Dy,c.MuFilter.Muon2Dz = -46.6*u.mm, 3964.1*u.mm, 128.6 *u.mm
           c.MuFilter.Iron3Dx,  c.MuFilter.Iron3Dy,   c.MuFilter.Iron3Dz    = -22.1*u.mm, 4009.6*u.mm, 128.6*u.mm
           # US3
           c.MuFilter.Muon3Dx,c.MuFilter.Muon3Dy,c.MuFilter.Muon3Dz = -46.6*u.mm, 4189.5*u.mm, 128.6 *u.mm
           c.MuFilter.Iron4Dx,  c.MuFilter.Iron4Dy,   c.MuFilter.Iron4Dz    = -22.1*u.mm, 4234.6*u.mm, 128.6*u.mm
           # US4
           c.MuFilter.Muon4Dx,c.MuFilter.Muon4Dy,c.MuFilter.Muon4Dz = -46.1*u.mm, 4415.6*u.mm, 128.6 *u.mm
           c.MuFilter.Iron5Dx,   c.MuFilter.Iron5Dy,  c.MuFilter.Iron5Dz    = -22.1*u.mm, 4459.6*u.mm, 128.6*u.mm
           # US5
           c.MuFilter.Muon5Dx,c.MuFilter.Muon5Dy,c.MuFilter.Muon5Dz = -46.6*u.mm, 4643.0*u.mm, 128.6*u.mm
           c.MuFilter.Iron6Dx,   c.MuFilter.Iron6Dy,  c.MuFilter.Iron6Dz    = -22.1*u.mm, 4684.6*u.mm, 128.6*u.mm
           # DS1
           c.MuFilter.Muon6Dx,c.MuFilter.Muon6Dy,c.MuFilter.Muon6Dz = -46.6*u.mm, 4869.6*u.mm - 1*u.mm, 128.6 *u.mm
           c.MuFilter.Iron7Dx,   c.MuFilter.Iron7Dy,  c.MuFilter.Iron7Dz    = -22.1*u.mm, 4923.6*u.mm, 128.6*u.mm

# fine adjustements
           #c.MuFilter.Muon8Dx = -4.7*u.mm - 4*u.cm
           #c.MuFilter.Muon7Dx = -47.1*u.mm + 0.74*u.cm
           #c.MuFilter.Muon1Dz = 146.6*u.mm - 2.3*u.cm
           #c.MuFilter.Muon2Dz = 136.6*u.mm - 1.5*u.cm
           #c.MuFilter.Muon5Dz = 127.7*u.mm + 0.25*u.cm
           #c.MuFilter.Muon7Dz = 127.7*u.mm + 0.25*u.cm
           #c.MuFilter.Muon8Dz = 127.7*u.mm + 0.56*u.cm

# from Scifi track alignment
           if tb_2023_mc :
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
             c.Scifi.LocM100, c.Scifi.LocM110 = 667.33*u.um, 429.73*u.um
             c.Scifi.LocM200, c.Scifi.LocM210 = 0.000*u.um, 51.80*u.um
             c.Scifi.LocM300, c.Scifi.LocM310 = 32.36*u.um, 803.10*u.um
             c.Scifi.LocM400, c.Scifi.LocM410 = 105.10*u.um, 371.07*u.um
             c.Scifi.RotPhiS10,c.Scifi.RotPsiS10,c.Scifi.RotThetaS10 =     0.00*u.mrad,    3.29*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS11,c.Scifi.RotPsiS11,c.Scifi.RotThetaS11 =     0.00*u.mrad,   -0.89*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS20,c.Scifi.RotPsiS20,c.Scifi.RotThetaS20 =     0.00*u.mrad,    0.00*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS21,c.Scifi.RotPsiS21,c.Scifi.RotThetaS21 =     0.00*u.mrad,    0.25*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS30,c.Scifi.RotPsiS30,c.Scifi.RotThetaS30 =     0.00*u.mrad,    0.93*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS31,c.Scifi.RotPsiS31,c.Scifi.RotThetaS31 =     0.00*u.mrad,    2.86*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS40,c.Scifi.RotPsiS40,c.Scifi.RotThetaS40 =     0.00*u.mrad,    0.15*u.mrad,    0.00*u.mrad
             c.Scifi.RotPhiS41,c.Scifi.RotPsiS41,c.Scifi.RotThetaS41 =     0.00*u.mrad,    3.13*u.mrad,    0.00*u.mrad
# Time alignment Scifi, T0 = station 0,  mat 0 
             c.Scifi.station1t,c.Scifi.station1H0t,c.Scifi.station1V0t =  0.000*u.ns,  0.000*u.ns,  1.535*u.ns
             c.Scifi.station2t,c.Scifi.station2H0t,c.Scifi.station2V0t =  1.246*u.ns,  0.000*u.ns,  -0.281*u.ns
             c.Scifi.station3t,c.Scifi.station3H0t,c.Scifi.station3V0t =  3.240*u.ns,  0.000*u.ns,  -0.913*u.ns
             c.Scifi.station4t,c.Scifi.station4H0t,c.Scifi.station4V0t =  1.750*u.ns,  0.000*u.ns,  -0.145*u.ns
