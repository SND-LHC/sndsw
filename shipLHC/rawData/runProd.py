import os,subprocess,time,multiprocessing,psutil
import pwd
import ROOT
ncpus = multiprocessing.cpu_count() - 2

""" input runlist
     for each run in runlist. look for number of partitions
     for each partition, start conversion job.
     check consistency of output file
     copy optional file to EOS
     run as many jobs in parallel as cpus are available
""" 

def delProcesses(pname):
    username = pwd.getpwuid(os.getuid()).pw_name
    callstring = "ps -f -u " + username
    status = subprocess.check_output(callstring,shell=True)
    for x in str(status).split("\\n"):
         if not x.find(pname)<0:
            first = True
            for pid in x.split(' '):
              if first:
                  first = False
                  continue
              if pid=='':continue
              os.system('kill '+pid)
              break 

class prodManager():

   def Init(self,options):
     self.Mext = ''  
     if options.FairTask_convRaw: 
        self.Mext = '_CPP'  
     self.options = options
     self.runNrs = []
   def count_python_processes(self,macroName):
      username = pwd.getpwuid(os.getuid()).pw_name
      callstring = "ps -f -u " + username
# only works if screen is wide enough to print full name!
      status = subprocess.check_output(callstring,shell=True)
      n=0
      for x in status.decode().split('\n'):
         if not x.find(macroName)<0 and not x.find('python') <0: n+=1
      return n
   def list_of_runs(self,macroName):
      lpruns = [[],[]]
      username = pwd.getpwuid(os.getuid()).pw_name
      callstring = "ps -f -u " + username
      status = subprocess.check_output(callstring,shell=True)
      for x in status.decode().split('\n'):
         if not x.find(macroName)<0 and not x.find('python') <0: 
            i = x.find('-r')
            k = x.find('-P')
            if i > 0:  
               r = int(x[i+3:].split(' ')[0])
               lpruns[0].append(r)
               if k>0: 
                  p = int(x[k+3:].split(' ')[0])
                  lpruns[1].append(r*10000+p)
      return lpruns

   def getPartitions(self,runList,path):
      partitions = {}
      for r in runList:
          directory = path+"run_"+str(r).zfill(6)
          dirlist  = str( subprocess.check_output("xrdfs "+options.server+" ls "+directory,shell=True) )
          partitions[r] = []
          for x in dirlist.split('\\n'):
             if x.find('.root')<0: continue
             ix = x.rfind('data_')
             if ix<0: continue
             partitions[r].append( int(x[ix+5:ix+9]) )
      return partitions

   def convert(self,runList,path,partitions={}):
      if len(partitions)==0: partitions = self.getPartitions(runList,path)
      for r in partitions:
         runNr   = str(r).zfill(6)
    # find partitions
         for p in partitions[r]:
             self.runSinglePartition(path,runNr,str(p).zfill(4),EOScopy=True,check=True)

   def getRunNrFromOffline(self,rMin=-1,rMax=99999):
      self.dqDataFiles = []
      hList = str( subprocess.check_output("xrdfs "+os.environ['EOSSHIP']+" ls /eos/experiment/sndlhc/www/offline",shell=True) )
      for x in hList.split('\\n'):
          if x.find('root')<0: continue
          run = x.split('/')[-1]
          if run.find('run')!=0: continue
          r = int(run[3:9])
          if r>=rMin and r<=rMax:  self.dqDataFiles.append(r)
   def runDataQuality(self,latest):
      monitorCommand = "python $SNDSW_ROOT/shipLHC/scripts/run_Monitoring.py -r XXXX --server=$EOSSHIP \
                        -b 100000 -p "+pathConv+" -praw "+path+" -g GGGG "\
                        +" --postScale "+str(options.postScale)+ " --ScifiResUnbiased 1 --batch --sudo  "
      if options.parallel>1: monitorCommand += " --parallel "+str(options.parallel)
      convDataFiles = self.getFileList(pathConv,latest,options.rMax,minSize=0)
      self.checkEOS(copy=False,latest=latest)
      # remove directories which are not completely copied
      for r in self.missing:
             if r in convDataFiles: convDataFiles.pop(r)
      # remove directories which are not fully converted
      rawDataFiles = self.getFileList(path,latest,options.rMax,minSize=10E6)
      self.RawrunNrs = {}
      for x in rawDataFiles:
             r =  x//10000
             if not r in self.RawrunNrs: self.RawrunNrs[r] = []
             self.RawrunNrs[r].append(x)

      orderedCDF = list(convDataFiles.keys())
      lpruns = self.list_of_runs('run_Monitoring')

      self.getRunNrFromOffline(latest)
      self.runNrs = {}
      for x in orderedCDF:
          r = x//10000
          if  r in self.runNrs:
              self.runNrs[r].append(x)
              continue
          if r in lpruns[0]: continue
          if r in self.dqDataFiles: continue
          self.runNrs[r] = [x]
          
      for r in self.runNrs:
           if r > options.rMax or r < options.rMin: continue # outside range
           if not r in self.RawrunNrs: 
               print('run not found in raw data',r)
               continue # file converted but not enough events
           if len(self.runNrs[r]) != len(self.RawrunNrs[r]): 
               print('run not complete',r)
               continue  # not all files converted.
           print('executing DQ for run %i'%(r))
           os.system(monitorCommand.replace('XXXX',str(r)).replace('GGGG',options.geofile)+" &")
           while self.count_python_processes('run_Monitoring')>(ncpus-2) or psutil.virtual_memory()[2]>90 : time.sleep(1800)

   def RerunDataQuality(self,runNrs=[],rMin=-1,rMax=99999):
      monitorCommand = "python $SNDSW_ROOT/shipLHC/scripts/run_Monitoring.py -r XXXX --server=$EOSSHIP \
                        -b 100000 -p "+pathConv+" -g GGGG "\
                        +" --postScale "+str(options.postScale)+ " --ScifiResUnbiased 1 --batch --sudo "
      if options.parallel>1: monitorCommand += " --postScale "+str(options.parallel)
      if len(runNrs) ==0:
         self.getRunNrFromOffline(rMin,rMax)
         runNrs = self.dqDataFiles
      for r in runNrs:
           print('executing DQ for run %i'%(r))
           if   r  < 4575:  geoFile =  "../geofile_sndlhc_TI18_V3_08August2022.root"
           elif r  < 4855:   geoFile =  "../geofile_sndlhc_TI18_V5_14August2022.root"
           elif r  < 5172:  geoFile =  "../geofile_sndlhc_TI18_V6_08October2022.root"
           elif r  < 5485: geoFile =  "../geofile_sndlhc_TI18_V7_22November2022.root"
           elif r  < 7357: geoFile =  "../geofile_sndlhc_TI18_V1_2023.root"
           else: geoFile =  options.geofile
           os.system(monitorCommand.replace('XXXX',str(r)).replace('GGGG',geoFile)+" &")
           time.sleep(20)
           while self.count_python_processes('run_Monitoring')>(ncpus-5) or psutil.virtual_memory()[2]>90 : time.sleep(300)

   def check4NewFiles(self,latest,rmax):
      rawDataFiles = self.getFileList(path,latest,rmax,minSize=10E6)
      convDataFiles = self.getFileList(pathConv,latest,rmax,minSize=0)
      orderedRDF = list(rawDataFiles.keys())
      orderedCDF = list(convDataFiles.keys())
      orderedRDF.sort(),orderedCDF.sort()

      for x in orderedRDF: 
           if x in orderedCDF: continue
           lpruns = self.list_of_runs('convertRawData')[1]
           if x in lpruns: continue
           r = x//10000 
           p = x%10000
           if r==4541 and p==38: continue   # corrupted raw data file
           print('executing run %i and partition %i'%(r,p))
           self.runSinglePartition(path,str(r).zfill(6),str(p).zfill(4),EOScopy=True,check=True)

   def runSinglePartition(self,path,r,p,EOScopy=False,check=True):
       while self.count_python_processes('convertRawData')>ncpus or self.count_python_processes('runProd')>ncpus: time.sleep(300)
       process = []
       pid = 0
       try:
           pid = os.fork()
       except OSError:
           print("Could not create a child process")
       if pid!=0:
          process.append(pid)
       else:          
         inFile = self.options.server+path+'run_'+ r+'/data_'+p+'.root'
         fI = ROOT.TFile.Open(inFile)
         if not fI:
           print('file not found',path,r,p)
           exit()
         if not fI.Get('event') and not fI.Get('data'):
           print('file corrupted',path,r,p)
           exit()
         command =   "python $SNDSW_ROOT/shipLHC/rawData/convertRawData.py  -r "+str(int(r))+ " -b 1000 -p "+path+" --server="+self.options.server
         if options.FairTask_convRaw:  command+= " -cpp "
         command += " -P "+str(int(p)) + " >log_"+r+'-'+p    
         print("execute ",command)
         os.system(command)
         if check:
          rc = self.checkFile(path,r,p)
          if rc<0: 
             print('converting failed',r,p,rc)
             exit()
         print('finished converting ',r,p)
         tmp = {int(r):[int(p)]}
         if EOScopy:  self.copy2EOS(path,tmp,self.options.overwrite)
         exit(0)
 
   def checkFile(self,path,r,p):
      print('checkfile',path,r,p)
      inFile = self.options.server+path+'run_'+ r+'/data_'+p+'.root'
      fI = ROOT.TFile.Open(inFile)
      if fI.Get('event'): Nraw = fI.event.GetEntries()
      else: Nraw = fI.data.GetEntries()
      outFile = 'sndsw_raw_'+r+'-'+p+self.Mext+'.root'
      fC = ROOT.TFile(outFile)
      test = fC.Get('rawConv')
      if not test:
          print('Error:',path,r,p,' rawConv not found')
          return -2       
      Nconv = fC.rawConv.GetEntries()
      if Nraw != Nconv: 
          print('Error:',path,r,p,':',Nraw,Nconv)
          return -1
      return 0

   def copy2EOS(self,path,success,overwrite):
     for i in success:
       r = str(i).zfill(6)
       for x in success[i]:
         p = str(x).zfill(4)
         outFile = 'sndsw_raw_'+r+'-'+p+self.Mext+'.root'
         tmpPath = os.environ['EOSSHIP']+pathConv+"run_"+r+"/sndsw_raw-"+p+".root"
         print('copy '+outFile+'  '+tmpPath)
         command = 'xrdcp '
         if overwrite: command+=' -f '
         os.system(command+outFile+'  '+tmpPath)
         os.system('rm '+outFile)

   def check(self,path,partitions):
     success = {}
     for r in partitions:
        success[r] = []
        runNr   = str(r).zfill(6)
        for x in partitions[r]:
           p = str(x).zfill(4)
           rc = checkFile(path,runNr,p)
           if rc==0: success[r].append(x)
     return success

   def getFileList(self,p,latest,rmax=999999,minSize=10E6):
      inventory = {}
      dirList = str( subprocess.check_output("xrdfs "+self.options.server+" ls "+p,shell=True) )
      for x in dirList.split('\\n'):
          aDir = x[x.rfind('/')+1:]
          if not aDir.find('run')==0 or aDir.find('json')>0:continue
          runNr = int(aDir.split('_')[1])
          if not runNr > latest: continue
          if runNr > rmax:       continue
          fileList = str( subprocess.check_output("xrdfs "+self.options.server+" ls -l "+p+"/"+aDir,shell=True) )
          for z in fileList.split('\\n'):
               jj=0
               tmp = []
               for o in z.split(' '):
                  if not o=='': tmp.append(o)
               if self.options.server.find('snd-server')>0:
                  jj = 0
                  k = z.rfind('data_')
                  if not k>0: continue
                  if not z[k+9:k+10]=='.': continue
                  first = False
                  for x in z.split(' '):
                     if x=='': continue
                     if not first and x=='sndecs': first = True
                     if first and not x=='sndecs': 
                       size = int(x)
                       break
               else:
                  k = max(z.find('data_'),z.find('sndsw'))
                  if not k>0: continue
                  size = int(tmp[3+jj])
               if size<minSize: continue
               theDay = tmp[4+jj] 
               theTime = tmp[5+jj]
               fname = z[k:]
               run = int(aDir.split('_')[1])
               if run>900000: continue     # not a physical run
               k = fname.find('.root')
               partition = int(fname[k-4:k])
               d = theDay.split('-')
               t = theTime.split(':')
               gmt = time.mktime( (int(d[0]), int(d[1]), int(d[2]),  int(t[0]), int(t[1]), int(t[2]), 0, 0, 0 ) )
               inventory[run*10000+partition] = [aDir+"/"+fname,gmt]
      return inventory

   def getNumberOfEvents(self,r):
# check for partitions
           dirlist  = str( subprocess.check_output("xrdfs "+options.server+" ls "+pathConv+"run_"+str(r).zfill(6),shell=True) )
           partitions = []
           for x in dirlist.split('\\n'):
                ix = x.find('sndsw_raw-')
                if ix<0: continue
                partitions.append(x[ix:])
           eventChain = ROOT.TChain('rawConv')
           for p in partitions:
                eventChain.Add(options.server+pathConv+'run_'+str(r).zfill(6)+'/'+p)
           return eventChain.GetEntries()

   def checkEOS(self,copy=False,latest=4361,rlast=999999):
       self.eosInventory = self.getFileList(path,latest,rlast)
       tmp = self.options.server 
       self.options.server = "root://snd-server-1.cern.ch/"
       self.daqInventory = self.getFileList('/mnt/raid5/data_online/',latest,rlast)
       self.options.server = tmp
       self.missing = {}
       for r in self.daqInventory:
            if not r in self.eosInventory:
               p = r%10000
               run = r//10000
               if not run in self.missing: self.missing[run]=[]
               self.missing[run].append(p)
       if copy:
          for r in self.missing:
               dirname ='run_'+str(r).zfill(6)
               for p in self.missing[r]:
                   filename = 'data_'+str(p).zfill(4)+'.root'
                   source = '/mnt/raid5/data_online/'+dirname+'/'+filename
                   target = path+dirname+'/'+filename
                   os.system("xrdcp -f "+source+" "+os.environ['EOSSHIP']+target)
       
   def getConvStats(self,runList):
     for run in runList:
       try: 
          f=ROOT.TFile.Open("sndsw_raw_"+str(run).zfill(6)+'.root')
          print(run,':',f.rawConv.GetEntries())
       except:
          print('problem:',run)
         
   def rawStats(self,runList):
      for run in runList:
         runNr   = str(run).zfill(6)
         r = ROOT.TFile.Open(os.environ['EOSSHIP']+path+"/run_"+runNr+"/data.root")
         if fI.Get('event'): raw = r.event.GetEntries()
         else:               raw = r.data.GetEntries()
         print(run,':',raw)

   def makeHistos(self,runList):
  # historic, used for H8 testbeam, use always DS tracks to survey US because of larger overlap
     for run in runList:
       command = "$SNDSW_ROOT/shipLHC/scripts/Survey-MufiScifi.py -r "+str(run)+" -p "+pathConv+" -g geofile_sndlhc_H6.root -c Mufi_Efficiency -n -1 -t DS"
       os.system("python "+command+ " &")
       while self.count_python_processes('Survey-MufiScifi')>ncpus:
          time.sleep(200)

   def mips(self):
  # historic, used for H8 testbeam, use always DS tracks to survey US because of larger overlap
     for run in runs:
       command = "Survey-MufiScifi.py -r "+str(run)+" -p "+pathConv+" -g geofile_sndlhc_H6.root -c mips"
       os.system("python "+command+ " &")
       while count_python_processes('Survey-MufiScifi')>multiprocessing.cpu_count()-2:
          time.sleep(200)

if __name__ == '__main__':

# use cases: H6, TI18
    from argparse import ArgumentParser
    parser = ArgumentParser()

    parser.add_argument("-r", "--runNumbers", dest="runNumbers", help="list of run numbers",required=False, type=str,default="")
    parser.add_argument("-P", "--production", dest="prod",       help="H6 / epfl / TI18"   ,required=False,default="TI18")
    parser.add_argument("-c", "--command", dest="command",       help="command", default=None)
    parser.add_argument("-o", "--overwrite", dest="overwrite",   action='store_true', help="overwrite EOS", default=False)
    parser.add_argument("-cpp", "--convRawCPP", action='store_true', dest="FairTask_convRaw", help="convert raw data using ConvRawData FairTask", default=False)
    parser.add_argument("--latest", dest="latest", help="last fully converted run", default=0,type=int)
    parser.add_argument("-A", "--auto", dest="auto", help="run in auto mode checking regularly for new files",default=False,action='store_true')
    parser.add_argument("--server", dest="server", help="xrootd server",default=os.environ["EOSSHIP"])
    parser.add_argument("-g", dest="geofile", help="geometry and alignment",default="geofile_sndlhc_TI18.root")
    parser.add_argument("--postScale", dest="postScale",help="post scale events, 1..10..100", default=-1,type=int)
    parser.add_argument("--parallel", dest="parallel",default=1,type=int)
    parser.add_argument("-rMin", dest="rMin",help="first run to process", default=-1,type=int)
    parser.add_argument("-rMax", dest="rMax",help="last run to process", default=99999,type=int)
    parser.add_argument("-p", dest="path", help="path to raw data",required=False,default="")
    parser.add_argument("-d", dest="pathConv", help="path to converted data",required=False,default="")
    parser.add_argument("--saveTo", dest="saveTo", help="output storage path", default="")
    parser.add_argument("-n", "--nEvents", dest="nEvents", help="number of events", default=-1,type=int)
    parser.add_argument("-s", "--nStart", dest="nStart", help="first event", default=0,type=int)

    
    options = parser.parse_args()
    M = prodManager()
    M.Init(options)

    runList = []
    if options.prod == "TI18":
       if options.server.find('eospublic')<0:
          path = "/mnt/raid5/data_online/" 
       else:
          path = options.path
          pathConv = options.pathConv
       
    elif options.prod == "reproc2022":
       path = "/eos/experiment/sndlhc/raw_data/physics/2022/"
       pathConv = "/eos/experiment/sndlhc/convertedData/physics/2022/"
       # first run = 4361
    elif options.prod == "H6":
       path     = "/eos/experiment/sndlhc/raw_data/commissioning/TB_H6/data/"
       pathConv = "/eos/experiment/sndlhc/convertedData/commissioning/TB_H6/"
       if options.runNumbers=="": 
           runList = [273,276,283,284,285,286,295,296,290,294,298,301,298,292,23,27,34,35,39,40, 41, 42, 43, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 123, 146, 165, 184] 
    elif options.prod == "epfl":
         path = "/eos/experiment/sndlhc/raw_data/commissioning/scifi_cosmics_epfl/data/"   
         if options.runNumbers=="": 
             runList = [3,8]
    elif options.prod == "test":
       path     = options.path
       pathConv = options.pathConv
    else:
        print("production not known. you are on your own",options.prod)

    if options.auto:
      while 1 > 0:
         M.check4NewFiles(options.latest,options.rMax)
         time.sleep(600)
      exit(0)

    if options.command == "DQ":
      while 1 > 0:
         M.runDataQuality(options.latest)  
         time.sleep(360)
      exit(0)
    elif options.command == "rerunDQ":
        runList = []
        for r in options.runNumbers.split(','):
            if r!= '': runList.append(int(r))
        M.RerunDataQuality(runList,options.rMin,options.rMax)

    elif options.command == "convert":
        for r in options.runNumbers.split(','):
            if r!= '': runList.append(int(r))
        M.convert(runList,path)    
   
    elif options.command:
       tmp = options.command.split(';')
       command = tmp[0]+"("
       for i in range(1,len(tmp)):
          if i<4: command+='"'+tmp[i]+'"'
          else:    command+=tmp[i]
          if i<len(tmp)-1: command+=","
       command+=")"
       print('executing ' + command )
       eval("M."+command)
