import numpy as np
from ROOT import TRotation, TVector3

class Abstract_DetElement:
    '''Detector element abstract class. Do not instantiate.
    Contains basic members and methods needed for every detector element
    '''
    
    def __init__(self,x,y,z,phi = 0,theta = 0,psi = 0):
        '''Constructor
            
        Initializes a detector element from the coordinates x,y and z of
        the geometric center of this element and three Euler angles using the
        x-convention.
        For more information about the x-convention implementation see the documentation
        of ROOT (e.g https://root.cern.ch/doc/master/classTRotation.html)
        
        Parameters
        ----------
        x : np.float64
            coordinate of the element's center in x direction
        
        y : np.float64
            coordinate of the element's center in y direction
            
        z : np.float64
            coordinate of the element's center in z direction
            
        phi : np.float64
            rotation of the element w.r.t the global z axis
            
        theta : np.float64
            rotation of the element w.r.t the global y axis
        
        psi : np.float64
            rotation of the element w.r.t the global x axis
        '''
        self._position = TVector3(x,y,z)
        self._rotation = TRotation()
        self._rotation.SetXEulerAngles(phi,theta,psi)
                
    def apply_translation(self,dx,dy,dz):
        '''Apply a translation to the detector element
        
        Applies a translation to the element
                
        Parameters
        ----------
        dx : np.float64
            Translation in x direction
            
        dy : np.float64
            Translation in y direction
            
        dz : np.float64
            Translation in z direction
        '''
        self._position[0] += dx
        self._position[1] += dy
        self._position[2] += dz
        
    def apply_rotation(self,dPhi,dTheta,dPsi):
        '''Apply a rotation to the detector element using the x-convention.
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
        self._rotation.RotateXEulerAngles(dPhi,dTheta,dPsi)
        
    def rotate_x(self,rad):
        """Rotate the detector element around the x-axis.
        This rotates the detector element an angle rad (in radians) w.r.t the global x-axis.
        This is an incremental rotation, hence it starts from the rotation the detector element has before
        calling this function.
        
        Parameters
        ----------
        rad : np.float64
            Rotation incrementation w.r.t the global x axis
        """
        self._rotation.RotateX(rad)

    def rotate_y(self,rad):
        """Rotate the detector element around the y-axis.
        This rotates the detector element an angle rad (in radians) w.r.t the global y-axis.
        This is an incremental rotation, hence it starts from the rotation the detector element has before
        calling this function.
        
        Parameters
        ----------
        rad : np.float64
            Rotation incrementation w.r.t the global y axis
        """
        self._rotation.RotateY(rad)     

    def rotate_z(self,rad):
        """Rotate the detector element around the z-axis.
        This rotates the detector element an angle rad (in radians) w.r.t the global z-axis.
        This is an incremental rotation, hence it starts from the rotation the detector element has before
        calling this function.
        
        Parameters
        ----------
        rad : np.float64
            Rotation incrementation w.r.t the global z axis
        """
        self._rotation.RotateZ(rad) 
            
    # Getter methods   
    def get_center_position(self):
        """Returns the position of the center of the detector element as a ROOT TVector3.
        
        Returns
        -------
        TVector3
            position of the center of this detector element
        """
        return self._position
    
    def get_rotation(self):
        """Returns the rotation of this detector element as ROOT TRotation object.
        For information on what you can do with the TRotation object see the ROOT documentation
        at https://root.cern.ch/doc/master/classTRotation.html
        
        Returns
        -------
        rotation
            TRotation object containing the rotation of the detector element.
        """
        return self._rotation
    
    
    # Helper methods for verbosity, debuggung and testing
    
    def print_rotation_matrix(self):
        """Print the rotational matrix (3x3).
        The matrix is:
        [ [xx, xy, xz] ,
          [yx, yy, yz] ,
          [zx, zy, zz] ]
        """
        print(self._rotation.XX(),self._rotation.XY(),self._rotation.XZ())
        print(self._rotation.YX(),self._rotation.YY(),self._rotation.YZ())
        print(self._rotation.ZX(),self._rotation.ZY(),self._rotation.ZZ())
        