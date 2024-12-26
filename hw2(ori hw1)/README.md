# CSCI 420 Assignment 2
## Name: Wu Kam Man. USC ID: kammanwu@usc.edu
## Basic Description
This project is able to simulate a real roller Coaster driving experience.

## Features
- **Catmull-Rom Spline Generation**: Generate smooth curves through a set of control points.
- **Frenet Frame Calculation**: Get tangents, normals, and binormals along splines.
- **Camera Movement**: Dynamic camera updates based on spline points, which simulates a physical law : gravity.
- **Gravity Simulation**: Add gravity to create a more realistic driving experience.
- **Shader Integration**: One set of shader for phong lighting and One set of shader for textureMapping.

## Extra Credit
- support: Subdivide Method In generating everything
- support: Real and Authentic Physic driving feeling (add gravity driving), you might even encouter surprising spin and shake during the drive 
- support: Track circular and close it with C1 continuity (this is in my extra spline file: goodRideExtra.sp)
- support: Better choice of Normal using sloan method.
