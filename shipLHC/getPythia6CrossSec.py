import multiprocessing as mp
import time
import os
import ROOT

ROOT.gInterpreter.Declare("""
#include "TPythia6.h"
// XSEC(0,3) the estimated total cross
// section for all subprocesses included (all in mb)

Float_t fixXsec(TPythia6& g) {
    Pyint5_t* p5 = g.GetPyint5();
    return p5->XSEC[2][0];
}
""")

def run_chunk(start_evt, n_events, pid, target, momentum, generator):
    for n in range(n_events):
        generator.GenerateEvent()
        generator.Pyedit(2)
        if n==(n_events-1):
          with open(results_filename,"a") as res:
             res.write(f"{pid} at {momentum} on {target} for {target_events} events: sigma= {ROOT.fixXsec(generator)}\n")
        with open(progress_filename,"w") as f:
             f.write(str(start_evt+n))

target_events = 1000000
n_generated = 0
progress_filename="progress.txt"
results_filename="results.txt"
mutype = {-13 : 'gamma/mu+', 13 : 'gamma/mu-' }
targets = ['p+', 'n']
list_of_momenta = [5., 10., 15., 20., 25., 50., 75., 100., 150., 200., 250., 300.,\
                  400., 500., 750., 1000., 1500., 2500., 5000., 7500., 10000.]

for pid in mutype:
  for target in targets:
    for mom in list_of_momenta:
      n_generated = 0
      if os.path.exists(progress_filename):
        os.remove(progress_filename) 
      print(f"Start for {pid} at {mom} on {target}. Init the Pythia6 run for {target_events} events.")
      myPythia = ROOT.TPythia6()
      myPythia.SetMSEL(2)
      myPythia.SetPARP(2,2)
      R = int(time.time()%900000000)
      myPythia.SetMRPY(1,R)
      myPythia.SetMSTU(11,11)
      myPythia.Initialize('FIXT',mutype[pid],target,mom)

      while n_generated < target_events:

        if os.path.exists(progress_filename):
            with open(progress_filename) as f:
              s=f.read()
              # Safety net. If the file is empty, we just iterate again starting from the
              # previous value.This means in the end one might have more than 1M generated events,
              # which is fine.Less is not fine.
              if s!="":  n_generated = int(s)

        n_events = target_events - n_generated
        print("(re)starting chunk at", n_generated)
        # Run the event generation as a separate process to monitor it.
        p = mp.Process(target=run_chunk,
                           args=(n_generated, n_events, mutype[pid], target, mom, myPythia))
        p.start()
        p.join(30)# allow 30 seconds
        if p.is_alive():
              print("Pythia got stuck ===> restarting")
              p.kill()
              p.join()
        #if all ran ok, increase the number of generated events to exit while loop
        else:
            n_generated += n_events

print("All done.")
