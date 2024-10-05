### Introduction

This page contains information about the network multiplayer feature.


### Direct connect

Networked multiplayer only works with Direct connect (all players need to specify IP).  
Can be local network or internet.  
If internet, all players need to have ports forwarded.  

There are no servers or game lists to get players, so to play  
you probaly need to enlist your friends to play with you.  


### Quick guide

  - You can host your own game by pressing the **Create game** button
    - Then others can connect directly to an existing IP:port by pressing **Direct connect**
  - After connecting, you can use in-game chat for text messages
  - Press **Ready** button in the game lobby tab and wait for others to become Ready too
  - Now the game will start when host presses Start button
  - Messages from chat, disconnect or winning will be shown during game at top

  - It is possible for host to continue game on other track (or restart on current):
    - When host presses **New game** (same button as Start) current game will end, peers will have shown menu
    - Host can now change track (and game settings) and after all peers have pressed **Ready**, game can start again

 
### Master (games) server (unavailable)

It's not working since years and likely won't be.  
Unless someone can host it on an IP that is not changing or better has a named domain. If so, please contact us.  

The list of available games will not be filled.  
Hence the last tab Server in Multiplayer has that big red cross.  
But buttons and list should be working, in case someone builds and starts the server program by themself.

Even when server was working, there was rarely a game in it.  
Stunt Rally is way less popular than e.g. Xonotic. You can help spreading its popularity.

   
### Troubleshooting

**Q:** I get a "Network error" message box when I try to join or create a game.

**A:** Maybe some other program is currently using the local port. Try a different one by changing it in the 'Settings' tab. Also make sure your firewall isn't blocking Stunt Rally - it needs permissions to both access the internet as well as listen for connections (i.e. permission to be a server).

   
**Q:** Other players can't connect to me

**A:** Most likely your router is blocking access. Configure it to forward the traffic coming to the Stunt Rally local port (check it from Multiplayer's 'Settings' tab) to your computer. You might find http://portforward.com/ helpful.

   
**Q:** I cannot connect to some players

**A:** This is the previous problem reversed - their firewall is misconfigured. If you have your router properly configured, the others should be able to connect to you and as such you don't need to connect to them anymore. Obviously this doesn't apply if you are the host.

   
**Q:** I get errors about protocol versions

**A:** This means that your version of Stunt Rally is incompatible with the master-server and/or other players (depending on with whom you are trying to communicate). Your best bet is to make sure you are running the latest release of SR. Otherwise, tell your fellow players or master-server administrator (that's us most likely) to upgrade.

   
**Q:** How can I change my nickname?

**A:** Go to the **Settings** tab under the Multiplayer main tab.

----

### Technical details

SR networking is based on the peer-to-peer architecture, meaning all players are connected to each other and there is no separate server for game.

The underlying protocol used is UDP, through ENet (networking library).  
SR builds its own layer on top of that, so using generic UDP sockets for communicating with SR or master server is not possible.

### Master (game) server

Stunt Rally **used** a lightweight master server for tracking available games.

Master server is a simple command-line server (can be daemonized on Linux)  
that accepts game announcements and passes a list of them to anyone who asks.  
Clients must update the games periodically or they will timeout in the master server.

Anyone can build and **host** a master server.  
Info also [here](../src/network/DesignDoc.txt).  
and its [code here](../src/network/master-server/main.cpp)
<del>it builds with CMake in SR repo</del> ToDo: fixme.

Players need to set the correct address and port in the Multiplayer - Settings tab in order to connect to it and use.

If master server works:
  - Host creates a game on Single Race - Multiplayer with 'Create game' button (just like for Direct connect).
  - Then others to join game: Go to the Single Race - Multiplayer, Server tab and press the 'Refresh list' button.
    - This will connect to master server and should list game(s)
  - Select a game you wish to join and press the 'Join game' button
