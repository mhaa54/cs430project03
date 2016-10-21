# CS 430 Project03 - Illumination
Project 03 for CS 430 Computer Graphics

In the previous project you wrote code to raycast mathematical primitives based on a scene
input file into a pixel buffer. In this project you will color objects based on the shading model
we discussed in class.

# Example to run the program

From the comand line in the order of :


![alt tag](https://github.com/mhaa54/cs430project02/blob/master/Example%20how%20to%20run%20the%20project.png)


# Specifically, these properties should be supported for lights:
<br />Position:The location of the light<br />
<br />Color The color of the light (vector)<br />
<br />Radial-a0 The lowest order term in the radial attenuation function (lights only)<br />
<br />Radial-a1 The middle order term in the radial attenuation function (lights only)<br />
<br />Radial-a2 The highest order term in the radial attenuation function (lights only)<br />
<br />Theta The angle of the spotlight cone (spot lights only) in degrees;<br />
<br />If theta = 0 or is not present, the light is a point light;<br />
<br />Note that the C trig functions assume radians so you may need to do a conversion.<br />
<br />Angular-a0 The exponent in the angular attenuation function (spot lights only)<br />
<br />Direction The direction vector of the spot light (spot lights only)<br />
<br />If direction is not present, the light is a point light<br />
# For objects, the properties from the last assignment should be supported in addition to:
<br />Diffuse_color The diffuse color of the object (vector)<br />
<br />Specular_color The specular color of the object (vector)<br />
<br />You may optionally include an object property, ns, for shininess<br />
