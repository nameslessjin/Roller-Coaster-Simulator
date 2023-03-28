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
Users can control the animation of the roller coaster through designated keys. <br>
"1": original speed <br>
"2": double of the original speed <br>
"3": half of the original speed <br>
"<": 10 steps backwards <br>
">": 10 steps forward <br>
"s": Switch rails texture on/off <br>
spacebar: start/pause

## Note
Examples of how to run the animation is provided in circle.sh, goodRide.sh, viper_whole.sh and viper.sh file in the root directory. <br>
viper_whole.sh and viper.sh are generating the same track.  However, viper generates the track with multiple splines and viper_whole generate the track with one spline.

## Extra Creidts
1. Render a T-shaped rail cross-section.  The head of T is on the right side of the rail.  I made it big so it is more visible.
<br>

2. Render double rail side by side
<br>

3. Track is circular and close with C1 continuity.  To see a good example of this condition, you can check circle.sp and star.sp.  Even if the u step is large size,
the spline will still remain circular and closed.  circle.sp and star.sp have been modified.  The last 3 control points which are the same as the first 3 control points in these files have been removed to ensure the consistency of this property in all spline files
<br>

4. Render a sky box.  It is texture-mapped.  The entire scene is enclosed into a cube.
<br>

5. Draw additional scene elements:  textured-mapped wooden cross bars and pillar stick for the rail.
<br>

6. Created tracks that mimic real world roller coasters, viper coaster.  Use the example from: </n>
https://rcdb.com/25.htm?p=73
<br>

7. Generate viper coaster tracks from several splines. (viper_up.sp, viper_turn_left.sp, viper_turn_right.sp, viper_second_hill.sp, viper_return.sp)
<br>

8. The spline drawn through this program uses recursive subdivision (vary step size to draw short lines) instead of using brute force (vary u with fixed step size)<br>

9. Modify the velocity with which our camera moves to make it physically realistic in terms of gravity.
<br>

## Extra features besides extra credits
1. Infinite looping.  The roller coaster will loop infinite without stop.
<br>
2. Roller coaster camera time/speed control, texture switch.  Described above.
<br>