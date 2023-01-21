import numpy as np
from DriftTube import DriftTube as Tube
from DetElement import Abstract_DetElement as DetElement
from ROOT import TRotation, TVector3

class DtModule(DetElement):
    '''Drift tube module: Physical assembly of 48 drift tubes
    Defines a module of drift tubes which resembles a physical assembly of
    drift tubes. These are arranged in four layers of twelve tubes each.
    '''
    
    def __init__(self,list_of_tubes,x,y,z,phi = 0,theta = 0,psi = 0):
        '''Constructor
            
        Initializes a drift tube module from the list of tubes
        that are part of this module as well as the coordinates x,y and z of
        the geometric center of this module and two angles of rotation w.r.t
        a global coordinate axis system.
        
        Parameters
        ----------
        list_of_tubes : list
            list of drift tubes that are part of this module
            
        x : np.float64
            coordinate of the module center in x direction
        
        y : np.float64
            coordinate of the module center in y direction
            
        z : np.float64
            coordinate of the module center in z direction
            
        phi : np.float64
            rotation of the module euler angle (x-convention)
            
        theta : np.float64
            rotation of the module euler angle (x-convention)
            
        psi : np.float64
            rotation of the module euler angle (x-convention)
            
        '''
        DetElement.__init__(self,x,y,z,phi,theta,psi) #python2
        #super().__init__(x,y,z,phi,theta,psi) #python3
        self._is_aligned = False
        #TODO consider deep copy
        self._list_of_tubes = list_of_tubes
       
    #Overwritten from DetElement
    def apply_translation(self,dx,dy,dz):
        '''Apply a translation to the whole module
        
        Applies a translation to the whole module (a.k.a for all tubes in the module)
        assuming that the relative positions of the contained drift tubes remain
        unchanged.
        
        Parameters
        ----------
        dx : np.float64
            Translation in x direction
            
        dy : np.float64
            Translation in y direction
            
        dz : np.float64
            Translation in z direction
        '''
        DetElement.apply_translation(self,dx,dy,dz) #python2
        # super().apply_translation(dx,dy,dz) #python3
        for tube in self._list_of_tubes:
            tube.apply_translation(dx,dy,dz)

    #Overwritten from DetElement
    def apply_rotation(self,dPhi,dTheta,dPsi):
        '''Apply a rotation to the whole module using the x-convention.
        Applies a translation to the whole module assuming that the relative
        orientations of the individual tubes in the module remain unchanged.
        This updates the tubes center positions first and then rotates them to match the
        module's orientation
        This rotates the detector element starting from its original rotation. Hence, it doesn't set
        the passed angles as new angles unless the element has zero rotation before.
        
        Parameters
        ----------
        dPhi : np.float64
            Rotation angle phi
        
        dThata : np.float64
            Rotation angle theta
            
        dPsi : np.float64
            Rotation angle psi
        '''
        DetElement.apply_rotation(self,dPhi,dTheta,dPsi) #python2
        #super().apply_rotation(dPhi,dTheta,dPsi) #python3

        # First: Update tube positions after module rotation, then update tube rotation to be the same as for module      
        for tube in self._list_of_tubes:
            self.__update_tube_pos_after_rotation(tube)
            tube.apply_rotation(dPhi,dTheta,dPsi)

    #Overwritten from DetElement
    def rotate_x(self,rad):
        """Rotate the whole module around the x-axis.
        This rotates the module an angle rad (in radians) w.r.t the global x-axis.
        This is an incremental rotation, hence it starts from the rotation the detector element has before
        calling this function.
        This updates the positions of the tube center positions after rotating the
        module as well.
        
        Parameters
        ----------
        rad : np.float64
            Rotation incrementation w.r.t the global x axis
        """
        DetElement.rotate_x(self,rad) #python2
        #super().rotate_x(rad) #python3
        for tube in self._list_of_tubes:
            self.__update__update_tube_pos_after_rotation(tube)
            tube.rotate_x(rad)

    #Overwritten from DetElement
    def rotate_y(self,rad):
        """Rotate the whole module around the y-axis.
        This rotates the module an angle rad (in radians) w.r.t the global y-axis.
        This is an incremental rotation, hence it starts from the rotation the detector element has before
        calling this function.
        This updates the positions of the tube center positions after rotating the
        module as well.
        
        Parameters
        ----------
        rad : np.float64
            Rotation incrementation w.r.t the global x axis
        """
        DetElement.rotate_y(self,rad) #python2
        #super().rotate_y(rad) #python3
        for tube in self._list_of_tubes:
            self.__update__update_tube_pos_after_rotation(tube)
            tube.rotate_y(rad)

    #Overwritten from DetElement
    def rotate_z(self,rad):
        """Rotate the whole module around the z-axis.
        This rotates the module an angle rad (in radians) w.r.t the global z-axis.
        This is an incremental rotation, hence it starts from the rotation the detector element has before
        calling this function.
        This updates the positions of the tube center positions after rotating the
        module as well.
        
        Parameters
        ----------
        rad : np.float64
            Rotation incrementation w.r.t the global x axis
        """
        DetElement.rotate_z(self,rad) #python2
        #super().rotate_z(rad) #python3
        for tube in self._list_of_tubes:
            self.__update_tube_pos_after_rotation(tube)
            tube.rotate_z(rad)
            
    def get_tubes(self):
        '''Get a list of tubes in this module
        Returns a list of drift tubes that are part of this module
        
        Returns
        -------
        list
            List of tubes contained in this module
        '''
        return self._list_of_tubes

    
    #Private helper functions
    
    def __update_tube_pos_after_rotation(self,tube):
        '''Private helper method! Do not call from outside!
        Update the center positions of a tube after the rotation of the module.
        Should be called after a rotation for each tube that is part of the module

        NOTE: This method is _NOT_ part of this class' interface

        Parameters
        ----------
        tube : DriftTube
            Tube thats center position shall be updated
        '''
        vec_tubecenter_modcenter = tube.get_center_position() - self._position
        new_tubecenter = self._rotation * vec_tubecenter_modcenter
        tube_translation = new_tubecenter - vec_tubecenter_modcenter
        tube.apply_translation(tube_translation[0],tube_translation[1],tube_translation[2])
        