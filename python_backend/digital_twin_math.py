import numpy as np
from scipy.interpolate import Rbf

def generate_3d_thermal_map(t1, t2, t3, t4, grid_res=20):
    """
    PI-SSET: Physics-Informed Sparse Spatial Edge-Digital Twin.
    Takes 4 vertical temperature readings and interpolates them into a 3D volume
    representing the server rack.
    """
    # Server rack dimensions (normalized 0 to 1)
    x = np.linspace(0, 1, grid_res)
    y = np.linspace(0, 1, grid_res)
    z = np.linspace(0, 1, grid_res)
    X, Y, Z = np.meshgrid(x, y, z)
    
    # We have 4 temperature probes at different heights (Z-axis)
    # Let's assume they are located in the center of the rack (X=0.5, Y=0.5)
    # Tier 1 (Bottom) to Tier 4 (Top)
    pts_x = np.array([0.5, 0.5, 0.5, 0.5])
    pts_y = np.array([0.5, 0.5, 0.5, 0.5])
    pts_z = np.array([1.0, 0.7, 0.4, 0.1])
    temps = np.array([t1, t2, t3, t4])
    
    # Also add "ambient" boundary conditions on the edges to force the heatmap 
    # to cool down towards the outer walls of the rack.
    amb = min(temps) - 1.0
    corners_x = [0, 0, 1, 1, 0, 0, 1, 1]
    corners_y = [0, 1, 0, 1, 0, 1, 0, 1]
    corners_z = [0, 0, 0, 0, 1, 1, 1, 1]
    corners_t = [amb] * 8
    
    all_x = np.concatenate([pts_x, corners_x])
    all_y = np.concatenate([pts_y, corners_y])
    all_z = np.concatenate([pts_z, corners_z])
    all_t = np.concatenate([temps, corners_t])
    
    # Use Radial Basis Function (RBF) interpolation to simulate thermal diffusion
    # 'multiquadric' or 'inverse' works well for heat equations
    rbf = Rbf(all_x, all_y, all_z, all_t, function='inverse')
    
    # Calculate the continuous 3D temperature field
    T_3D = rbf(X, Y, Z)
    
    return X, Y, Z, T_3D
