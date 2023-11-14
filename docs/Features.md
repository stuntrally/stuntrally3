_This page lists available features in Stunt Rally 3 game and editor._  
_Todo:_ - means it's not present or not yet fully restored from old SR 2.x

## Game Modes

- 👤 One Player
  - ❔ How to play
    - 👈 Driving school - 6 lessons, replays with subtitles
    - Many basic test tracks for practicing and fun
  - 🏁 Single Race
    - 👻 Ghost drive (orange) - chase your best time vehicle on track
    - Track's ghost (green) - best drive for track, a green ghost car ES, on all tracks
    - 📽️ Replays - save your drive and watch it from other cameras later
  - Tutorials - few short and easy tracks for start, with various track elements
  - 🏆 Championships - usually long series of tracks.  
    Easier - Drive to get higher score, if too low repeat a stage.
  - 🥇 Challenges - few tracks, quite difficult to pass, game already set up.  
    Challenging - Drive with no mistakes to win bronze, silver or gold prize.
- 👥 More Players
  - 🪟 Split Screen - 2 to 4 players on 1 screen (requires a good GPU or reduced graphics options)
  - 🌎 Multiplayer - over internet or in local network, in-game chat. See Wiki, no game server, needs direct connect.

## Gameplay 🏁

- 🏞️ 226 **Tracks**
  - in 40 **sceneries**, great variety: from nature based, to alien, space and surreal
  - over 10 hours to drive all, just once
- **Stunt** elements: obstacles, jumps, loops, pipes or very twisted road
- 🌊 Drives through water or mud areas (safe, won't get stuck)
- 📦 Dynamic objects on tracks (barrels, boxes, etc), can be hit
- 🚗 25 **Vehicles**
  - 16 cars (Hyena or Fox are best for rally)
  - few extreme with bigger wheels, or: 3, 6 and 8 wheeled 🚌
  - 2 motorbikes 🏍️
  - 3 hovering spaceships 🚀, 1 bouncy sphere 🔘
- 💨 Boost - key to apply rocket boosters to speed up vehicle, works in air and underwater too
- ⏪ Rewind - hold key to go back in time (after hit or mistake)
- Simulation modes: Easy (beginner) and Normal
- 🔨 Simple vehicle damage %, doesn't affect driving until 50%

## Input

- 🎮 Game controllers support (gamepad, steering wheel, joystick etc.)
- ⌨️ Input configuring (reassign keys, change sensitivity)

## Gui and HUD ⏱️

- 🚦 Pacenotes - signs above road telling about:
  - turns (and their difficulty), long turns (more white), terrain bumps,
  - loops (few types), jumps (with required speed, match by color) and on pipe changes
- 🎗️ Racing line (Trail)
  - shows best way to drive, red colors for braking
- 🌍 Minimap, can be zoomed and rotated, _Todo:_ terrain and border toggleable
- Driving aids
  - 🔝 arrow to next check point (HUD top), _Todo:_ fix for split screen
  - next check point beam (on track)
  - track completion percentage %
  - time difference and current points shown on each checkpoint
- 📃 Detailed list of tracks with:
  - 🖼️ preview image and minimap (click to toggle zoom)
  - 📊 stats like: rating, difficulty, length, width, jumps, loops, pipes, obstacles and much more
  - driveability % estimate for vehicle on track (low means unfit)
- ⏲️ Several gauge types (for engine rpm and vehicle speed)

## Graphics

- 8 Graphics presets for quick change to match hardware performance
- Many graphics options to tweak each feature individually
- PBS materials with normalmaps, roughness and fresnel
- Terrain triplanar mapping
- Reflections and fresnel (for Vehicles, water etc)
- _ToDo:_ Refraction for water

------------------------------------------------------------------------------

## Track Editor 🏗️🏞️

- Spline based road - no predefined sets or tiles, giving road absolute freedom in 3D, smoothed between points
- Real-time editing of road points and their parameters - quick adjusting with keys or mouse to see how it looks
- Changing all of track parameters in editor (no other tools needed), after each save track is ready to play in game
- Terrain
  - Edit modes: Deform, Smooth (flatten), Filter, Set Height
  - 169 brush presets for editing. Possible to edit own
  - Terrain generator with many parameters
- In-editor **Help** page with all key shortcuts and mouse actions listed
- Console tab, listing track errors, warnings and very simple hints for quality when saving
- _Old:_ docs page and video tutorials available [here](Editor.md)
- No undo - using quick save and quick load

## Vehicle editing 🚗

- In game vehicle editor
  - allows faster tinkering with simulation parameters, collision, etc
  - aids creating new ones, page [here](VehicleEditing.md)
- Many graphs (visualizations) of key simulation values
  - gearbox and torque curve, suspension, differentials etc
- Tire functions (Pacejka's Magic Formula) editing _while_ driving
- Tire grip cricles visualization
