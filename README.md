# Simulating a Roller Coaster

https://github.com/nameslessjin/Roller-Coaster-Simulator/assets/42518630/c1623575-03ed-4973-a204-44808c3db0a8



## Introduction
In this project, we used Catmull-Rom splines along with OpenGL core profile shader-based lighting and texture mapping to create a roller coaster simulation.  We implemented two shaders: one for texture mapping (to render the ground), and one for Phong shading (to render the roller coaster rail).  The simulation will run in a first-person view, allowing the user to "ride" the coaster in an immersive environment.

### Splines Creation - level 1
We used Catmull-Rom to create splines with given control points.  Due to the nature of Catmull-Rom, the first and the last points in the given control points are ignored.  

### Roller Coaster Riding - level 2
This simulation will run in a first-person view, allowing the user to "ride" the roller coaster.  We calculate Frenet Frame (tangent, normal, binormal) through Sloan's method.  Sloan's method decides each coordinate system using a function of the previous one, to ensure continuity.  The tangent can be calculated from the derivative of the spline function.  We set the first N0=unit(T0xV) and B=unit(T0xN0).  In here, V is an arbitrary vector.  We pick V as (0, 1, 0).  The following normal and binormal can be calcuated with N1=unit(B0xT1) and B1=unit(T1xN1).

### Rail Cross Section - level 3
We use the normals and binormals we obtained from level 2 to create cross section for rail.  For each position on the splines, we generate 4 points on topleft, top right, bottomleft, bottomright.  We then forms the cross section with these 4 points using GL_TRIANGLES.  The color of each points is the same as the normal of each triangle.  The cross section is formed by 4 surfaces.

### Ground Texture - level 4
We add a ground plane in the direction of up z-axis.  The ground is texture-mapped with an image.  In this work, we write a vettex and fragment shader to do texture mapping and then create a new pipepline in the CPU code

### Phong Shading - level 5
In this part, we computes Phong shading and use 1 directional light to light the scene.  We modified the basic vertex and fragment shaders to perform Phong Shading, and adjust the Phong shading properties to render the lighting effect properly.

## Extra Features

1. Render a T-shaped rail cross section
2. Render a Double Rail
3. Made the track circular and closed it with C1 continuity
4. Additional Scene Elements: <br>
    - Texture-mapped wodden cross bars
    - Texture-mapped wodden pillars
    - Texture-mapped wodden rails
5. Render a sky-box
6. Create tracks that mimic real world roller coaster.  Use the example from: https://rcdb.com/25.htm?p=73
7. Generate track from several sequences of splines <br>
    use viper.sh in root dir or viper.txt in hw2-starterCode.  The viper roller coaster is made of viper_up.sp, viper_turn_left.sp, viper_turn_right.sp, viper_second_hill.sp, viper_return.sp
8. Draw splines using recursive subdivision
9. Render environment in a better manner:<br> 
    When track texture switch is on, I render the texured rails with pow(light, 1.2) on the texture instead of base Phong lighting.  It provides higher contrast on the track texture.  When track texture switch is off, the lighting on the track is base Phong lighting
10. Modify velocity with which the camera moves


Additional Features:
1. Infinite looping.  The roller coaster will loop infinite without stop
2. Roller coaster camera time/speed control, texture switch.  Described below: <br>
Users can control the animation of the roller coaster through designated keys. <br>
"1": original speed <br>
"2": double of the original speed <br>
"3": half of the original speed <br>
"<": 10 steps backwards <br>
">": 10 steps forward <br>
"s": Switch rails texture on/off <br>
"r": Reset the position and start recording <br>
spacebar: start/pause
