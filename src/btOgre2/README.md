# BtOgre2 - Simple Bullet-Ogre 2.x connection


BtOgre2 is a simple Bullet <-> Ogre connection made for Ogre 2.1 and up

This project is Free and Open Source, licensed under the term of the zLib license agreement (see LICENSE for more informations).

## Build and usage instructions

### Building BtOgre2

You will need CMake to build the library. and at least a C++11 complient compiler.

Depending on your installation, you may need to have to define theses variables for CMake (notably on MS Windows):

 - `OGRE_HOME` : where your Ogre **2.1** SDK is located
 - `BULLET_ROOT` : where your Bullet directory is located
 
 The preprocessor will not permit to build the library against a non compatible version of Ogre.

Then you'll be able to use your favorite compiler and toolchain to build this project.
(Was tested with Visual Studio 14 (2015) x64, Visual Studio 15 (2017à x64), and GCC 6.3.1)

Then you can build and install (e.g. `make; sudo make install`) the library.

### Using BtOgre2

In /CMake/ you will find a CMake module that will permit you to "try" to find an installed version of BtOgre2. You can help it by defining the CMake Cache variable `BtOgre21_ROOT` with the PATH to your BtOgre2 installation.

This will populate the usual variables for libraries with CMake. See the /demo/ directory for an example on how to use it.

## Attribution of original work

This is a forked project from BtOgre, originally created by nikki93 (as of his github name), and take root from a fork by nyxkn.

The goal is to reorganise the source code of this project, and also do some cleaning, and make it's functionality available to Ogre 2.1 users.

## Changes made from the original verison of BtOgre

 - Modified to compile against Ogre 2.1, (and will work with upcomming versions)
 - Modernized the array location 
 - Added an overload to construct a static shape converter from an Ogre::v1::MeshPtr
 - Motion states moves "derivated coordinates" instead of the "regular" ones. This prevent unsynchronising graphics and physics when you have object parenting
 - The static *mesh to shape converter* has overload to use v2 meshes and Items, instead of just v1 meshes or Entity object
 - Class implementation are not in headers
 - Debug drawer re-implemented for Ogre V2 by using a v2 manual object
   - The debug drawer now supports every mode of debug drawing Bullet can offer, and does it with the proper colors
   - The debug drawer uses an HLMS Unlit datablock created at run time the first time you call it, and set vertex colors on each points of each lines
   - The color of the line is multiplied by a factor the user can set to accomodate HDR rendering pipeline and the way the user wants to deal with color spaces and gamma correction.

## Changes planned

  - Reorganise the classes into multiple files
  - Revisit the animated *mesh to shape converter* but right now Ogre v2 animations and v1 animations co-exist in a weird state
  - Try to implement something for soft body physics.
  - Try to make sure nothing's break if you change the origin of the scene. Ogre support that now by moving the root node. This could cause thing to go quite wrong in the Physics <-> Graphics communication
  - Integrate a motion state that don't actually move the node until told to do so by the user, for multithreaded uses cases. (See Ogre 2.1 samples)

--- 

## Current state of this project

Consider this at a "beta" phase. The *core* functionality works. You can setup a *static mesh-to-shape converter* form an Ogre Entity or Item, generate a rigidbody you can give to Bullet, and use a *motion state* to let bullet move objects according to the simulated laws of physics (gravity, collision, forces and impultions applied, all that good stuff...) 

Currently, I can't recomment to anybody to actually use this thing. But it's working. I'm using it in my game engine. Just generate a project via CMake and build the static library. ;-)

In the present state, it is relying on Ogre v1 mesh for the animated mesh part. I actually never used this part of BtOgre myself, but I plan to extend that part to Ogre v2 too.
