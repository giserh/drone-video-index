This repository includes the source code of drone video metadata processing and drone video indexing. 


Folder "drone-metadata-generator":
1. Calculate quadrilateral-shaped aerial-FOVs from drone video raw sensor-metadata 
(e.g., azimuth, pitch, roll angles).

2. Generate a large dataset from a small real dataset with the similar distribution. 


3. Generate queries on drone videos. 



Folder "aerial-fov-mbr-rtree":
Implementation of r-tree indexing for aerial-fovs.



Folder "aerial-fov-tetrartree":
Implementation of the proposed TetraR-tree for aerial-fovs.
