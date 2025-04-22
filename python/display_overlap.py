import ROOT
import SndlhcGeo
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Circle
import matplotlib.cm as cm
import matplotlib.colors as colors
import argparse


def load_scifi(geo_file):
    snd_geo = SndlhcGeo.GeoInterface(geo_file)
    lsOfGlobals = ROOT.gROOT.GetListOfGlobals()
    scifi = snd_geo.modules['Scifi']

    return scifi

def getFiberWeight(FibresMap):

    # create a fiber_weight list with 3 column fID weight pos
    fiber_weight = {}
    for fiber in FibresMap:
        fID = fiber.first
        for channel in fiber.second:
            chanID = channel.first
            weight = channel.second[0]
            pos = channel.second[1]
            #print(fID, chanID, weight, pos)

            if fID not in fiber_weight:
                fiber_weight[fID] = [weight, pos]
            else:
                fiber_weight[fID][0] += weight
                if pos != fiber_weight[fID][1]:
                    #print(f"changing channel {chanID} pos from {fiber_weight[fID][1]} to {pos}")
                    fiber_weight[fID][1] = pos
                
    return fiber_weight

def plot_overlap(scifi, plot_outfile):
    sGeo = ROOT.gGeoManager
    scifi.SiPMOverlap()
    scifi.SiPMmapping()

    SiPMmap = scifi.GetSiPMmap()
    FibresMap = scifi.GetFibresMap()
    print("GetSiPMmap",len(SiPMmap))
    print("GetFibresMap",len(FibresMap))

    #print("number of fibers in map",len(FibresMap))
    #print(SiPMmap)

    fiber_weight = getFiberWeight(FibresMap)
    #print(fiber_weight)
    print("number of fibers in map", len(fiber_weight))
    
    # check all volumns
    # volume_list = sGeo.GetListOfVolumes()
    # for vol in volume_list:
    #     name = vol.GetName()
    #     #if "SiPM" in name:
    #     print(name)

    y_values = []
    z_values = []

    fig, ax = plt.subplots(figsize=(10, 10))
    SiPMmapVol = sGeo.FindVolumeFast("SiPMmapVol")
    for sipm in  SiPMmapVol.GetNodes():
        t0 = sipm.GetMatrix().GetTranslation()[0]
        t1 = sipm.GetMatrix().GetTranslation()[1]
        t2 = sipm.GetMatrix().GetTranslation()[2]

        y0 = t1
        z0 = t2

        DX = sipm.GetVolume().GetShape().GetDX()
        DY = sipm.GetVolume().GetShape().GetDY()
        DZ = sipm.GetVolume().GetShape().GetDZ()

        #print("sipm t0, t1, t2", t0, t1, t2)

        rect = Rectangle((y0 - DY, z0 - DZ), DY*2, DZ*2, edgecolor='blue', facecolor='none')
        ax.add_patch(rect)

        y_values.extend([y0 - DY, y0 + DY])
        z_values.extend([z0 - DZ, z0 + DZ])

        #break

    ScifiHorPlaneVol = sGeo.FindVolumeFast("ScifiHorPlaneVol1")
    imat = 0 
    norm = colors.Normalize(vmin=0, vmax=1.4)  
    cmap = cm.get_cmap('RdYlGn')
    for mat in  ScifiHorPlaneVol.GetNodes():    # 3 mats
        mat_t0 = mat.GetMatrix().GetTranslation()[0]
        mat_t1 = mat.GetMatrix().GetTranslation()[1]
        mat_t2 = mat.GetMatrix().GetTranslation()[2]

        #print(f"mat {imat+1} t0,t1,t2", mat_t0, mat_t1, mat_t2)
        for fiber in mat.GetVolume().GetNodes():
            fiber_t0 = fiber.GetMatrix().GetTranslation()[0]
            fiber_t1 = fiber.GetMatrix().GetTranslation()[1]
            fiber_t2 = fiber.GetMatrix().GetTranslation()[2]
            #print("fiber t0,t1,t2", fiber_t0, fiber_t1, fiber_t2)

            y0 = fiber_t1 + mat_t1
            z0 = fiber_t2 

            fiber_radius =  fiber.GetVolume().GetShape().GetDX()

            fID = fiber.GetNumber()%100000 + imat*1e4
            if fID in fiber_weight:
                f_weight = fiber_weight[fID][0]
            else:
                f_weight = 0
            color = cmap(norm(f_weight))
            circ = Circle((y0, z0), radius=fiber_radius, edgecolor='red', facecolor=color, alpha=0.8)
            ax.add_patch(circ)

        imat+=1



    print("SiPM Y axis position limit:", (min(y_values), max(y_values)))
    print("SiPM Z axis position limit:", (min(z_values), max(z_values)))

    xlim = (min(y_values)-0.1, max(y_values)+0.1)
    ylim = (-0.09, 0.09)

    fig_width = (xlim[1] - xlim[0]) * 10
    fig_height = (ylim[1] - ylim[0]) * 10
    fig.set_size_inches(fig_width, fig_height)

    ax.set_xlim(*xlim)
    ax.set_ylim(*ylim)
    ax.set_aspect('equal')
    ax.set_xlabel("Y axis (cm)")
    ax.set_ylabel("Z axis (cm)")

    # add 0.081 to y axis ticker
    yticks = list(ax.get_yticks())  
    yticks.append(0.081)           
    yticks = sorted(set(yticks))    
    ax.set_yticks(yticks)
    
    plt.grid(True)
    plt.savefig(plot_outfile, dpi=300) 
    print(f"plot saved to {plot_outfile}")


def main():
    parser = argparse.ArgumentParser(description="Plot overlap for SciFi SiPM.")
    parser.add_argument("--geo", type=str, required=True, help="Path to the geometry ROOT file.")
    parser.add_argument("--output", type=str, required=True, help="Path to save the output plot.")
    args = parser.parse_args()

    geo = args.geo
    output_path = args.output

    scifi= load_scifi(geo)

    plot_overlap(scifi, output_path)

if __name__ == "__main__":
    main()

# example usage:
# python display_overlap.py --geo /eos/user/z/zhibin/snd_MC/testbeam/6layer/pion/0/geofile_full.PG_211-TGeant4.root --output plot/overlap_testbeam_6layer.pdf 
# python display_overlap.py --geo /eos/user/z/zhibin/snd_MC/testbeam/7layer/pion/0/geofile_full.PG_211-TGeant4.root --output plot/overlap_testbeam_7layer.pdf 
# python display_overlap.py --geo /eos/experiment/sndlhc/convertedData/physics/2022/geofile_sndlhc_TI18.root --output plot/overlap_TI18.pdf

