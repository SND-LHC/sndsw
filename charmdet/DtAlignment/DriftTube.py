from DetElement import Abstract_DetElement as DetElement
from ROOT import TVector3
import shipunit as u

class DriftTube(DetElement):
    """Representation of a drift tube. It holds a position containing its
    geometric center position as well as its rotation as a ROOT TRotation object.
    Additionally, it holds its unique detector ID.    
    """
    def __init__(self,detId,x,y,z,phi = 0,theta = 0,psi = 0):
        '''Constructor. Initializes a drift tube.
        In terms of rotation the rotational angles 0,0,0 correspond to the
        tube in vertical orientation.
        
        Parameters
        ----------
        id : int
            Unique detector ID of this drift tube
            
        x : np.float64
            coordinate of the tube's center in x direction
        
        y : np.float64
            coordinate of the tube's center in y direction
            
        z : np.float64
            coordinate of the tube's center in z direction
            
        phi : np.float64
            rotation of the tube euler angle (x-convention)
            
        theta : np.float64
            rotation of the tube euler angle (x-convention)
            
        psi : np.float64
            rotation of the tube euler angle (x-convention)  
        '''
        DetElement.__init__(self,x,y,z,phi,theta,psi) # python2
        #super().__init__(x,y,z,phi,theta,psi) #python3
        
        #TODO: Read length from TGeo
        if(int(detId / 10000000) < 3):
            self._length = 110 * u.cm
        else:
            self._length = 160 * u.cm
            
        self._ID = detId
        
            
    def wire_end_positions(self):
        '''Returns the end positions of the wire in a global reference system.
        
        Returns
        -------
        TVector3
            Top position of the wire
            
        TVector3
            Bottom position of the wire
        '''
        global_y_axis = TVector3(0,1,0)
        l = self._length / 2
        tube_axis = self._rotation * global_y_axis
        tube_axis.SetMag(l)
        #TODO: test if rotation is implemented correctly
        x_top = self._position[0] + tube_axis[0]
        y_top = self._position[1] + tube_axis[1]
        z_top = self._position[2] + tube_axis[2]
        
        x_bot = self._position[0] - tube_axis[0]
        y_bot = self._position[1] - tube_axis[1]
        z_bot = self._position[2] - tube_axis[2]
        
        return TVector3(x_top,y_top,z_top), TVector3(x_bot,y_bot,z_bot)
