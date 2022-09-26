#!/usr/bin/env python
# -*- coding: latin-1 -*-

import ROOT,os
import shipunit as u
from ShipGeoConfig import ConfigRegistry
detectorList = []

def configure(run,ship_geo,Gfield=''):
	# -----Create media-------------------------------------------------
	if hasattr(run,'SetMaterials'):  run.SetMaterials("media.geo")  # Materials
	
	# -----Create geometry----------------------------------------------
	cave= ROOT.ShipCave("CAVE")
	cave.SetGeometryFileName("caveWithAir.geo")
	detectorList.append(cave)

	if "Floor" in ship_geo:
		floor = ROOT.Floor()
		for parName in ship_geo.Floor:
			parValue = eval('ship_geo.Floor.'+parName)
			floor.SetConfPar("Floor/"+parName, parValue)
		detectorList.append(floor)
	
	"""if "EmulsionDet" in ship_geo:
		EmulsionDet = ROOT.EmulsionDet("EmulsionDet",ROOT.kTRUE)
		for parName in ship_geo.EmulsionDet:
			parValue = eval('ship_geo.EmulsionDet.'+parName)
			EmulsionDet.SetConfPar("EmulsionDet/"+parName, parValue)
		detectorList.append(EmulsionDet)
	
	if "Scifi" in ship_geo:
		Scifi = ROOT.Scifi("Scifi", ROOT.kTRUE)
		for parName in ship_geo.Scifi:
			parValue = eval('ship_geo.Scifi.'+parName)
			Scifi.SetConfPar("Scifi/"+parName, parValue)
		detectorList.append(Scifi)
	
	if "MuFilter" in ship_geo:
		MuFilter = ROOT.MuFilter("MuFilter",ROOT.kTRUE)
		for parName in ship_geo.MuFilter:
			parValue = eval('ship_geo.MuFilter.'+parName)
			MuFilter.SetConfPar("MuFilter/"+parName, parValue)
		detectorList.append(MuFilter)
	"""
	if "Magnet" in ship_geo: #Magnet is only available in advSND geometry files
		Magnet = ROOT.Magnet("Magnet",ROOT.kTRUE)
		for parName in ship_geo.Magnet:
			parValue = eval('ship_geo.Magnet.'+parName)
			Magnet.SetConfPar("Magnet/"+parName, parValue)
		detectorList.append(Magnet)
	
	if "AdvTarget" in ship_geo: #AdvTarget is only available in advSND geometry files
		AdvTarget = ROOT.AdvTarget("AdvTarget",ROOT.kTRUE)
		for parName in ship_geo.AdvTarget:
			parValue = eval('ship_geo.AdvTarget.'+parName)
			AdvTarget.SetConfPar("AdvTarget/"+parName, parValue)
		detectorList.append(AdvTarget)
	
	if "AdvMuFilter" in ship_geo: #AdvMuFilter is only available in advSND geometry files
		AdvMuFilter = ROOT.AdvMuFilter("AdvMuFilter",ROOT.kTRUE)
		for parName in ship_geo.AdvMuFilter:
			parValue = eval('ship_geo.AdvMuFilter.'+parName)
			AdvMuFilter.SetConfPar("AdvMuFilter/"+parName, parValue)
		detectorList.append(AdvMuFilter)

	print(detectorList)
	detElements = {}
	if hasattr(run,'SetMaterials'):  
		for x in detectorList:
			run.AddModule(x)
	# return list of detector elements
		for x in run.GetListOfModules():
			detElements[x.GetName()]=x
		else:
			for x in detectorList:
				detElements[x.GetName()]=x
		return detElements
