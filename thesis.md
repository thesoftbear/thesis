# Abstract

# Introduction

# Related Work
### Particle Data Structures
### Voxel Cone Tracing
### Ambient Occlusion

# Hashed Uniform Grid
Algorithm:
1. calculate cell of each particle
2. sort particles into cells -> 1 atomic per particle (better than 8+ with scattering)
3. gather contribution per cell from cells in particle size radius

# Ambient Occlusion
### Ray Tracing
idea + hemisphere sampling
ray sphere intersection math

### Voxel Cone Tracing
cone tracing idea + cone approximation with voxels

# Implementation
### Hashed Uniform Grid
### Ray Tracing
### Voxel Cone Tracing

# Evaluation
### Results
1. Qualitativ
VCT on standard resolution grid - ray tracing on particle data -> Fehlerquellen auswerten:
VCT on high resolution grid -> difference = voxelization error
Ray tracing on voxel grid -> difference = cone tracing error

2. Quantitativ
Scattering vs Gathering Performance
??? VCT vs Ground Truth Performance ??? (Wie soll man das messen? Wann ist Ground truth gut genug?)

### Discussion

### Future Work

# Conclusion

# References