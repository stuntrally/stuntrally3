## Vehicles

*Post from forum.*  
Future plans for vehicles, models list etc.


## 🚗Cars, new vehicles - done and to do

Old [topic here](https://forum.freegamedev.net/viewtopic.php?f=80&t=18526) with previous 4th list.

Continues here for SR 3.x and beyond.


Rather low priority, unless we get too bored with current vehicles again and go into this.

I'll be short this time:
- I have a list of vehicles I'd like in SR3, my [collection](https://sketchfab.com/cryham/collections) in now [on sketchfab](https://skfb.ly/oDrzD).
- I'd focus on Sci-Fi, and concept models, not based on real vehicles, nor cars.

## General info

If you're a 3D model artist or have some knowledge in Blender, and want to help here then please reply or email me.  
If you have skills to follow this process, I'd be also open for other vehicles.

Otherwise the list is already big and I'm not interested in any other vehicles. Unless I missed some decent CC models, ready for game, on other sites.

If you want to learn, then e.g. here's a playlist [tutorial](https://www.youtube.com/watch?v=ILqOWe3zAbk&list=PLa1F2ddGya_-UvuAqHAksYnB0qL9yWDO6&index=2).  
There are also SR3 docs/ pages about cars: [VehicleModeling](VehicleModeling.md) and [VehicleEditing](VehicleEditing.md)  
and our repo with blend files for current vehicles.  


Models in my collection still need some work (like 2 days to even 3 weeks):  

- make them low poly for game (remove subdivisions, decimate when necessary)
- UV unwrap, texturing (best is all non-body parts on 1 texture)
- interior (if none, have to make a simple one), can use these assets: one and two
- exporting and testing


## Posts

Lastly, post if you have comments about done cars, especially after testing them.

- CryHam · 4 Dec 2023

As for code and simulation, I would like to create a racing hover craft, which model I started some time ago. And this code I'd like to use also for new generation of spaceships and sphere that could levitate easily with orientation from normal (esp. in pipes) and handle driving in pipes too. I was thinking of just removing wheels but keeping their ray casts and suspension. Will try someday.

- CryHam · 23 Mar 2024

So I did that. And now we have the first:
Sci-Fi hovercraft car

Hover car is (will be in SR 3.1) easy and fast like spaceships.

Using 4 anti-gravity forces, from .car suspension in wheel locations.

This way they can handle pipes well, don't care about grip since no tires. Can fly up and fall slower too.
Video here

model from here.
WIP Stunt Rally 3.1 upcoming hover car
youtu.be
Testing latest antigravity technology for Stunt Rally racing.Will be in SR 3.1. Is right now in git.

- CryHam · 24 Mar 2024

I'm also adding drones. Same anti-gravity hovering. Just small, round and very fast.

Model is this here.

- CryHam · 24 Mar 2024

We will also soon have this racing hovercraft, just fan powered.

Model here
❤️

- CryHam · 30 Mar 2024

So we have now this thing. 2nd drone Q3, crazy fast.

Right now 1.5 sec for 0 to 200 km/h, top speed 915 😱, I like it.

Is a new challenge to fly this, makes all else slow.

Also has great detail few 4k textures. Model here.

- CryHam · 30 Mar 2024

Now we have 3 hover cars: R1, R2, R3. Those very simple models.

Models went into my collection vehicles-in-stunt-rally-already

I have 2 more models, higher quality in my collection vehicles-todo-for-stunt-rally. They need more time, so probably for later.  

[Vehicles - in Stunt Rally already](https://sketchfab.com/cryham/collections/vehicles-in-stunt-rally-already-b478078f853644349fe0758b3c448170)  
[Vehicles - ToDo for Stunt Rally](https://sketchfab.com/cryham/collections/vehicles-todo-for-stunt-rally-327a2dd7593f47c7b97af6b806a60bb8)  

- CryHam · 31 Mar 2024

And now also 2nd racing hovercraft

Is the one I started some time ago, now finished by combining with stuff from 1st.

Still WIP, I don't have a sound for them and no rotating propeller.

- CryHam · 1 Apr 2024

New fast car YG (like Y7 but shorter and agile). This model.

This is enough for the next release 3.1.

BTW I also made AppImage, seems working, will upload soon to test for all. I just have some unfinished stuff ATM.

- CryHam · 7 Apr 2024

A small update, about simulation. I also added more text with color in Vehicle tab for new types.

Hovercrafts (H1, H2) do hover over fluids, but if you turn it to side (with flip Q or W) then you can force it to go underwater. So I was actually able to drive H2 on Atlantis. But Track Drivability will now show such tracks (marked as u1 in tracks.ini as having part needing underwater) as 0% i.e. undrivable.

Then Hover cars (R1, R2, R3) and drones (Q1, Q3) with handbrake steer more and drop down. Hover cars get buoyancy and go into mud and drones ignore fluids completely (that's the best antigravity).

This should work and be in Track Drivability but I can't test each combination value. So if you see any bad value for vehicle - track pair, let me know. It's a problem if Drivability shows high % but you can't drive a vehicle on a track.
