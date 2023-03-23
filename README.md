# CSCI 420 Programming Assignment 2: Simulating a Roller Coaster

## Introduction
In this assignment, we will use Catmull-Rom splines along with OpenGL core profile shader-based lighting and texture mapping to create a roller coaster simulation.  We will implement two shaders: one for texture mapping (to render the ground), and one for Phong shading (to render the roller coaster rail).  The simulation will run in a first-person view, allowing the user to "ride" the coaster in an immersive environment.

## Features
### Splines Creation - level 1
We use Catmull-Rom to create splines with given control points.  Due to the nature of Catmull-Rom, the first and the last points in the given control points are ignored.  

### Roller Coaster Riding - level 2
This simulation will run in a first-person view, allowing the user to "ride" the roller coaster.  We calculate Frenet Frame (tangent, normal, binormal) through Sloan's method.  Sloan's method decides each coordinate system using a function of the previous one, to ensure continuity.  The tangent can be calculated from the derivative of the spline function.  We set the first N0=unit(T0xV) and B=unit(T0xN0).  In here, V is an arbitrary vector.  We pick V as (0, 1, 0).  The following normal and binormal can be calcuated with N1=unit(B0xT1) and B1=unit(T1xN1).

### Rail Cross Section - level 3
We use the normals and binormals we obtained from level 2 to create cross section for rail.  For each position on the splines, we generate 4 points on topleft, top right, bottomleft, bottomright.  We then forms the cross section with these 4 points using GL_TRIANGLES.  The color of each points is the same as the normal of each triangle.  The cross section is formed by 4 surfaces.

### Ground Texture - level 4
We add a ground plane in the direction of up z-axis.  The ground is texture-mapped with an image.  In this work, we write a vettex and fragment shader to do texture mapping and then create a new pipepline in the CPU code

### Phong Shading - level 5
In this part, we computes Phong shading and use 1 directional light to light the scene.  We modified the basic vertex and fragment shaders to perform Phong Shading, and adjust the Phong shading properties to render the lighting effect properly.

### Animation Control
Users can control the animation of the roller coaster through designated keys.
"1": original speed
"2": double of the original speed
"3": half of the original speed
"<": 10 steps backwards
">": 10 steps forward
spacebar: start/pause


## Extra Creidts
1. Render a T-shaped rail cross-section.  The head of T is on the right side of the rail.  I made it big so it is more visible.
<br><br>

2. Render double rail side by side
<br><br>

3. The spline drawn through this program uses recursive subdivision (vary step size to draw short lines) instead of using brute force (vary u with fixed step size)<br><br>

4. Track is circular and close with C1 continuity.  To see a good example of this condition, you can check circle.sp and star.sp.  Even if the u step is large size,
the spline will still remain circular and closed.  circle.sp and star.sp have been modified.  The last 3 control points which are the same as the first 3 control points in these files have been removed to ensure the consistency of this property in all spline files
<br><br>