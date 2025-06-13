## Multiplayer server

*Topic from forum.*  
Discussion about: new Masterserver for multiplayer (testing..).

Recent version from git repo has this set up in game-default.cfg.  
If not using that, info how to set it up in last post, bottom of page.


## Posts

- Kris · 24 Jun 2024

Are there plans to set up a new master server for multiplayer again?

We recently discussed playing the new Stunt Rally 3 on [onFOSS](https://onfoss.org/), but it seems like online multiplayer is not quite in a state that this would be easily doable.

- CryHam · 25 Jun 2024

I am open for contributors in this area and good ideas. I don't have plans myself because I don't know any good place to run such master / games server.

For me SR3 multiplayer works. But I only test it on 1 PC. So I am unsure about rest. And there is [an issue](https://github.com/stuntrally/stuntrally3/issues/28) with LAN not working for some. IDK, I got not much info.

Whole **hosting topic** is also for discussion:
Since a while I'm not happy about tuxfamily, it's down like 3 times a year for a week or so. That's far worse than any hosting which usuall guarantees 99% or so.  
I did even consider paying but honestly I'm confused about all those weird prices. It's like 70% off for 1st year just to fool you with the price, then regular, and after 5 or 10 years I've seen the price (for domain at least) goes crazy high. IDK.

Still, there would be a lot of work probably, to move SR and my website to some other host.
Here are some "free" but not really  
https://www.hostingadvice.com/how-to/best-web-hosting-for-nonprofits/

So far I got info about STK, seems to be using this  
https://www.namecheap.com/  
https://www.whois.com/whois/supertuxkart.org  
I like it most. Seems cheap but those costs get like 10x bigger after few years or so, hard to tell, same for domain costs.  
https://www.namecheap.com/wordpress/

and Speed dreams, info there  
https://www.speed-dreams.net/en/major-changes-in-the-speed-dreams-project/

seems they moved to https://forge.a-lec.org/  
but that's in French so IDK even.

Also I got doubts about paying for SR and my website, they don't get much money if at all.

My website is really slow too, IDK why I need like 4 sec to refresh usually. At least when editing but I've tested and it seems slow outside too.
I emailed tuxfamily about it, and also asked about newer php, but they didn't reply even. And old php is a known issue, seems they don't care since years.

- Kris · 25 Jun 2024

We could probably host a multiplayer masterserver here on our infrastructure. I already run the ingame chat for the OpenHV game, so this kind of thing is definitely in scope if it isn't too maintenance heavy. But I think for online multiplayer to work well, there needs to be an improvement that not all participants have to manually open ports in their firewalls for it to work. Maybe there could be a headless mode to run a game server that is only a spectator in the game?

As for your website, why not move it to https://codeberg.page/ ? They don't support php though, so you would need to switch to a static site builder like Hugo.

- EP · 25 Jun 2024

Good day all, I do not know, neither am I a user of these things right now. But just one day learning about many OS initiative I saw e.g. Jami, though for video chat. There is no centralized server in these cases. https://jami.net/ similar peer 2 peer I guess possible for gaming to. I did not use it by the way. But this peer 2 peer stuff could perhaps be something the check out. Regarding hosting also it is possible to run once own hosting, or decentralized solutions.

- MxtAppz · March 10

Hi all,

I just read that and I'd be happy to host both the masterserver (lobby) and an official dedicated game server. I've been doing that for almost a year with other open source games and had ~99% uptime and players reported no lag issues. I've been playing SR3 since 2018 or so and I think that's what the game lacks, and maintaining the servers means almost no extra work for me, so if you're interested I'm open.

- CryHam · March 15

Hi thanks, that would be great. Do you have a constant IP address or a domain name, that we could then use in SR settings? For SR to reach your PC running that small program called game or "master server". I have built it recently, I probably need to test it more, and understand why it didn't work for me, IDK if it can't on same PC as game, or which ports does it need to use etc.  
Does your PC have Linux or Windows? For this master server program.  
I should build it and send it to you so we could test. I think I didn't include it in SR releases.  

Before I was thinking of moving to https://forge.a-lec.org/  
Which probably could run some php game server like for Speed Dreams.  
But this it is a very long process still, and it first needs this done from: [data](data.md).

And since recent release I've put SR developing on hold. Right now I'm developing [CaveExpress](https://github.com/mgerhardy/caveexpress).

- MxtAppz · March 16

Hi newly, I have a sticky IP (the IP has been the same for about 6-7 months) but we cannot really rely on that because of course I don't know when it's going to change. However, I use DDNS solutions to overcome this (so I don't have to pay for a domain and still has the very same effect). I use a headless Linux and I run all the servers daemonized with systemd, because it's very handy and allows fine-grained control about the process running in the container.

Of course, you can send me the program (or give me the instructions to compile it so I can automatize this every time a new version is released), or if you prefer, I can give you SSH access to the server so you can set everything up, check logs, update, reconfigure it or whatever you need to do without going through me.

- CryHam · March 18

Sounds great.  
Sorry for delays I'm lately quite consumed by developing [caveexpress](https://github.com/mgerhardy/caveexpress).  
From what I remember:  
there is an option in our CMakeLists.txt  
to build master/game server:  
option(BUILD_GAME_SERVER  
and this small program should build and run. Not sure if it works well.  
I only once tested it, but on same PC, maybe that's why it didn't for me.  
Its sources are here.  
There is a DesignDoc.txt from Tapio (he wrote it and our multiplayer too), has some info.  
So far I remember from when it was running, there were like 1 or 2 games played daily, maybe, or none..  

- MxtAppz · March 23

Hey there, so I managed to build the server and have it up working, and connected (by setting the IP on my laptop). I knew it was successfully connected by the logs, because I couldn't tell that by looking at the GUI (I guess if it displays no error then everything is OK). FYI, I did it using 2 public IPs (the server had an IP and the client another one, so perhaps that's the issue in your setup, using the same PC for both the server and the client as you mentioned above). Regarding the server program IDK if there's some way for it to enable stdout logging to check whether the server received the client connection but I managed to figure it out using netstat.

I also tried to record a video for you to know what I got but it captured a serious GUI problem (I'll open a github issue as well and attach the video too because as you'll see it's hard to explain what happens, basically the window minimizes itself almost every time it receives focus and it blinks and flashes as if the monitor/screen was broken, and this condition alternates as the window loses and grabs focus [using GNOME + Wayland on Linux]).

If you need me to try anything else with the server don't hesitate to tell me.
sr3.webm 15.5 MB

- CryHam · March 23

Okay thanks for testing.
I replied in [github issue](https://github.com/stuntrally/stuntrally3/issues/49), I don't think there is much we can change on that hardware.
Could you share the IP:port on which the SR master/game server is running (if it is).
I could try to add it to SR3 settings, create a game, anch check if the multiplayer games list works.

- CryHam · March 27

Hi all.
Thanks to MxtAppz, there is a master/game server running now.

To use it enter in SR3 **Multiplayer tab Server**, under master server:
```
IP: mxtappz.ddns.net
Port: 4243
```

or if you prefer to edit **game.cfg** instead, find and put these values there:
```
master_server_address = mxtappz.ddns.net
master_server_port = 4243
```

I think it works fine. I see it working, shows a game when I created it. Which is what it does. Then other player can get/refresh list, click a game and press join.
(it's easier than knowing and typing IP in direct connect)

But so far I wasn't able to start a game like so (on same PC). One game shows it's listening on 4242, other that it's connecting to 4242 and that's it, nothing happens.
I think I did forward ports 4242-4244 (both tcp and udp) okay on my router. And also added them to iptables on my Debian 12 (info [how here](https://itslinuxfoss.com/open-port-debian-12/)).
No idea why it doesn't work. I should also be able to direct connect to my public IP (not local) after port forwarding (opening). But this also fails same way.

If anyone wants to test and post if the server works for you that'd be great.
