import ROOT

def align_modules(dt_modules,track):
    # 1. set labels
    keys = dt_modules.keys()
    alignables = [] * len(keys)
    for i in range(len(keys)):
        alignables[i] = dt_modules[keys[i]]
    # 5 parameters since drift tubes are not sensitive to translation along wire when
    # read out on one side only
    labels = set_labels(alignables, 5)
        
    return 

def align_single_tubes(dt_modules,track):
    return

def set_labels(list_of_alignables,n_parameters):
    ''' Calculate labels for each parameter of each alignable element
    
    In Millepede, each alignable element (e.g a single drift or a module of drift tubes)
    is aligned with respect to a set of parameters that can be chosen freely.
    For example, the set of parameters can be three translations and three rotations.
    As each alignable element is aligned for these parameters, e.g for 10 modules of drift
    tubes we can use 10 * 6 parameters. Each of these needs a unique, numerical label.
    This function calculates labels for each parameter and returns them in a list.
    
    Parameters
    ----------
    list_of_alignables: list
        A list that contains the alignable objects
        
    n_parameters: int
        The number of alignment parameters per alignable element
        
    Returns
    -------
    list
        A list of labels for all parameters. This is ordered as follows:
        alignable 1: parameter 1
        alignable 1: parameter 2
        ...
        alignable 1: parameter N
        alignable 2: parameter 1
        ...
    '''
    labels = [] * n_parameters * len(list_of_alignables)
    if n_parameters < 10:
        multiplier = 10
    else:
        multiplier = 100
    for i in range(len(list_of_alignables)):
        for j in range(n_parameters):
            labels[i+j] = multiplier * i + j
            
    return labels
        