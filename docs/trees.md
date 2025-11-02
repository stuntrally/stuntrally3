## Trees

*Post from forum.*  

## 🌳Trees to do

A copy from old forum. So I could edit, mark here etc.

Trees are in SR3 just like other objects. Just export and put mesh and textures in trees2/,  
add in presets.xml, new material in all.material.json (copy similar) and test in editor.

They need **LODs** (like 3 at least, let's say up to 6). This is *very* important.  
LODs are done with that OgreMeshTool or could even be set (and work) in export from Blender.  
If we had a tree with 30k tris it needs to reduce it very quickly. Less issue for 10k trees but still needs. And for a 3k tree that's already too low so whatever.  

LODs need testing and this is tedious.  
Testing is done with 1st slider on Gui tab Detail to 0 (lowest lod bias).  
They can't look bad (too separated tris, lost shape, etc). If they do then total % tris reduced is too high.  
But with good (e.g. 20k) trees LODs need to allow us to have many of them far without too high triangles count (F11 to test).

## Tree Generators

So firstly there is a great tool, (better than what I used to generate trees2/) new trees generator:  
https://drajmarsh.bitbucket.io/tree3d.html  
good but not complex, probably needs just textures from elsewhere.  
See my comment *below*, I actually go with Blender addon `Sapling Tree Gen` now, has much more options.

----
## Collection

Below I have a huge list of links
- We need for jungle first, this could be hard not many good models, low poly.
- Palms also, many available. Few bushes even too.
- Some other too, could be easy

So let's try first those with good tris count, 5-10k (could go up to 20-30k with great LODs).  
Other way you could just use those links for leaves textures and do it with that generator above.  

Be extra *cautious*: check for licenses and real artists. Avoid trolls or bots copying stuff around idiotically, there are some on sketchfab.

### Links
For bark textures our favorite Yughues did many long ago:

https://opengameart.org/content/free-materials-pack-30-redux  
https://opengameart.org/content/free-materials-pack-22-redux  
https://opengameart.org/content/tiny-weeds  
https://opengameart.org/users/yughues?page=8

----- jungle - badly needed to replace tree.07 and tree.09

18k good for jungle? do mesh etc maybe vary trunks  
https://sketchfab.com/3d-models/tree-animate-f0f9eb5e6c104bbb8e1f41c97019e6f2

41k 3x drop animation, do mesh etc good for jungle?  
https://sketchfab.com/3d-models/tree-wind-system-87a2022bd8ad4c85b4d11825143e4e77

4k 2x good too low?  
https://sketchfab.com/3d-models/low-poly-and-realistic-trees-for-tropical-areas-538245447f80470c8f8f720d534ee12b

30k good jungle needs some variation, work  
https://sketchfab.com/3d-models/jungle-tree-46f83ec5f6c04abf9d509c1070f67d1e

----- too high

100k 3x good, but reduce a bit on branches  
https://sketchfab.com/3d-models/small-trees-18445272ace448ceb1429a96f5ecc31b

130k way too high poly, awesome tree  
https://sketchfab.com/3d-models/mango-tree-4b186052228d43d8b3fbb63213677de8

124k jungle leaves too high poly  
https://sketchfab.com/3d-models/jungle-tree-b760147faadf41eabb2bb6cb85274d8a

111k too high, great tree reduce possible? remove vines? we need 30k or less  
https://sketchfab.com/3d-models/chinese-banyan-ficus-microcarpa-2a0dbcdf8f5d48f5ad79987c7a8170ce

----- very good tropical shrubs

https://opengameart.org/content/tropical-shrubs  
https://opengameart.org/content/bushes  
https://opengameart.org/content/tiny-weeds  

1.6M! ultra hq, awesome tree, difficult, needs redoing leaves and decimate  
https://sketchfab.com/3d-models/ceiba-pentandra-tree-9142b8f06e384f1c9554c9b10da79d2a

282.3k way too high poly but awesome tree  
https://sketchfab.com/3d-models/realistic-hd-blue-jacaranda-940-e40bd12a6a1a4eb7a71bf2e8c2e671af

102k 6x idk trunks bad  
https://sketchfab.com/3d-models/fantasyassets-vol3-trees-and-bushes-74b55ab3337e4a8996d00a862d4a4311

----- few at once

17k good 3x  
https://sketchfab.com/3d-models/elm-trees-free-low-poly-352c29c013434d6585e74332699310e2  
https://sketchfab.com/3d-models/beech-trees-free-low-poly-f59546058ee44e73920e79fa78614f21

texture bad  
https://sketchfab.com/3d-models/birch-trees-free-low-poly-a90b8cca57b44f5492e796cf94d64e80

13k idk low  
https://sketchfab.com/3d-models/birch-trees-de6263e5400b4a52a795decf84f06575

7k idk  
https://sketchfab.com/3d-models/realistic-tree-model-104028c8350a4612b84b5e1c6b409bd4

3k meh, small, texture good  
https://sketchfab.com/3d-models/hill-side-tree-30b8b40b99874d158138fb5633ad84da

license ok? ue only or sth?  
https://sketchfab.com/NextSpring/models

------ palms - we can have more :)

3k palm good  
https://sketchfab.com/3d-models/free-game-ready-palm-e8c0ea53410947b0b7f20d33702f4098

2k tall good  
https://sketchfab.com/3d-models/coconut-tree-low-poly-8c5d6b661b2f4c37834d87cd187eb907

cd good  
https://opengameart.org/content/free-palm-treez-v3

37.1k palms  
https://sketchfab.com/3d-models/trees-2bea5ce17978453187efb2ae0de5d2d4

10k good, bit too high, reduce  
https://sketchfab.com/3d-models/date-palm-11acf710e6c149daa8d6fb8cdc5d087f

7k cactus good  
https://sketchfab.com/3d-models/cactus-2-saguaro-e7ba6d448e2c44ce89c716fa71cbb716

8k ok license?  
https://sketchfab.com/3d-models/realistic-palm-tree-model-vol1-24bc514ac5e04e56a97fcb9b555aa67d  
https://sketchfab.com/3d-models/realistic-palm-tree-model-vol2-56e55c30c03b4b6993441e0a15b87a19

7k  
https://sketchfab.com/3d-models/coconut-palm-26e787f2ff2e4c0fb004c3b0210805a3

idk many  
https://sketchfab.com/3d-models/tropical-plants-pack-m02p-2f093afb792742438f0f7ba7eaab90f0

3k good  
https://sketchfab.com/3d-models/joshua-tree-cb75d80490ae4d698179d63561cccfef

4k ok  
https://sketchfab.com/3d-models/curly-palm-00f2b57dd0e844edbeb116034fa471ec

3k 2x idk  
https://sketchfab.com/3d-models/2-palm-low-poly-a7a6ed3dac31493d8280e4bc6b1bfd9c

3x idk  
https://www.blendswap.com/blend/17074

idk  
https://www.blendswap.com/blend/20393

----- regular etc

14k 3x good?  
https://sketchfab.com/3d-models/oak-trees-d841c3bcc5324daebee50f45619e05fc

6k willow, forest  
https://sketchfab.com/3d-models/willow-tree-7bd70b487fae4f3eb70d4e69394e97b4

8k elm idk too low?  
https://sketchfab.com/3d-models/elm-tree-cf70c356a8a54df8a007ec74be0241ab

1.6k low quality?  
https://sketchfab.com/3d-models/game-ready-tree-2caa3f89aef64a5795721b0c2e6b3904

5k good pine  
https://sketchfab.com/3d-models/spruce-a50a5df3164246a5af97992cec33a143

4k cypres ok?  
https://sketchfab.com/3d-models/bald-cypress-feb56a05db3546d390d0957e03cf48a3

5k idk  
https://sketchfab.com/3d-models/pine-tree-d45218a3fab349e5b1de040f29e7b6f9

1k tiny  
https://sketchfab.com/3d-models/lowpoly-tree-b562b2e9f029440c804b4b6d36ebe174

2k idk low?  
https://sketchfab.com/3d-models/tree-d365d69d0cd14259b037dc67f9ce5141

29k few too low?  
https://sketchfab.com/3d-models/tropical-trees-4f7dc2339fa1483a9d4b880559ab83bf

https://sketchfab.com/3d-models/african-plain-trees-c19db0db391f424f975a895a810aeca3

32k idk meh tall  
https://sketchfab.com/3d-models/tree-66a8d0ef9b49415a9eaf6af216cb9bce

33k too high  
https://sketchfab.com/3d-models/tree-for-games-f91d3c3c527d47fdb217c291e4c7df4b

129k way too high, cool tree  
https://sketchfab.com/3d-models/macadamia-tree-f70a95844f3d40b9b7fac241a71f9cd8

----- dry, empty

3k ok dry  
https://sketchfab.com/3d-models/tree-a2a6237a270840e198cc7db1c47f1ef7

20k 5x  
https://sketchfab.com/3d-models/trees-eed7470843504aa592514554a6100fbc

22k reduce? dry  
https://sketchfab.com/3d-models/ancient-tree-960fb92d7192458daeaf4282cc6193e2

12k oak  
https://sketchfab.com/3d-models/oak-tree-6468dd4d3eb240ef902b9057d9913606

6k fantasy idk  
https://sketchfab.com/3d-models/a-fantasy-tree-bc7547be2e554125b3917bd2b80b8866

shrub texture good no trunk?  
https://sketchfab.com/3d-models/cliff-shrub-for-terrain-7a0f2bf2982c4ff8bacc67856fdbfae2

----- spring flowering trees, not that needed now

3k spring  
https://sketchfab.com/3d-models/sakura-c562e993e5ba411f8edd4b65ef0069d0

30k spring 2x decimate trunks  
https://sketchfab.com/3d-models/sakura-trees-free-0c4079868ab247b9aab88cb25ce1ba31

8k ok?  
https://sketchfab.com/3d-models/tree-cb3003a0129846a8802ebd973bda0286

156.8k 3x leaves too high poly  
https://sketchfab.com/3d-models/cherry-blossom-trees-f69be55d2e4f4f73b568ebb185bd8496

https://sketchfab.com/3d-models/maple-trees-9f51e08e56cd4e44b35fd02a3ad8d3a6

81k 5x too high poly?  
https://sketchfab.com/3d-models/cottonwood-tree-pack-c1de6e55a7f541639f750b6467ac24f3

82k way too high trunk  
https://sketchfab.com/3d-models/treea-d44222f5e9994ad6b4a975951c0ada44

200k way too high decimate?  
https://sketchfab.com/3d-models/realistic-high-poly-tree-457c8c70eb5a45649d5b42b9bc9c3337

----- hq cc0 idk may be good

https://polyhaven.com/a/jacaranda_tree  
https://polyhaven.com/a/island_tree_02  
https://polyhaven.com/a/island_tree_01  
https://polyhaven.com/a/anthurium_botany_01  
https://polyhaven.com/a/pine_tree_01  
https://polyhaven.com/a/fir_tree_01  

!78k texture good decimate trunk  
https://sketchfab.com/3d-models/high-quality-tree-36-53623598e75844b696abf2b94566afa5

60k idk decimate trunk, replace texture?  
https://sketchfab.com/3d-models/high-quality-tree-66-221b8943cc514162a05be24b7ba35430

some old vid in blender, are more  
https://www.youtube.com/watch?v=rN2CYXVKH0s


----
## Posts

*Forum posts history below, from this topic.*

- CryHam · 22 Jan 2024

Ok so far I did test a few new trees I made in the new [web generator](https://drajmarsh.bitbucket.io/tree3d.html).

So ugh SR3 doesn't really handle HQ trees well. I thought we could have 20k or 30k tris but seriously (for jungle) it's better to not go over 8k, unless for very rare trees. As soon as a tree is often, dense this rises total triangles rendered very much.

So yeah I'm gonna aim at 7k or so for normal, big trees, and like 4k for medium etc. And I'm already using just square (4) for branches max hexagonal (6) steps. Broad trunk will need manual refines (in Blender) near bottom for good look.

Another important thing are LODs. Well so far I got them only meh looking. But needed. IDK if possible or how to conveniently generate meshes separately and join them into 1 mesh LODs. Would likely give better results. Right now the auto generation just drops leaves (too much) and they quickly look like stick figures, like half of what the trees were.

So yeah trees are pretty delicate and crucial for Fps and render performance.

I'll continue with this. Will be doing jungle first, tree.07, tree.09 will drop first. I'll do few different trees to replace varying on sceneries (probably own for jungle, forest, savannah etc).

- CryHam · 26 Jan 2024

Well, web tree generator is okay but doesn't have that many options. But is surely the fastest way of making simple trees, well still need to import obj in Blender scale etc and export.

I found like 3 **addons for Blender**.

1. Sapling Tree Gen, it is already in Blender, need to just enable. Works fine and has plenty options.  
[Video here](https://www.youtube.com/watch?v=jMDRc4hJwvA) and [another](https://www.youtube.com/watch?v=uTtqdRAM_00). Only thing is that you can't change parameters after creating, they're gone. And needs some extra material setup to actually see textures in Blender before.
There is also an [Improved version](https://blenderartists.org/t/improved-sapling-tree-generator/642704), I'm not sure yet.
2. MTree [modular-tree-addon](https://blender-addons.org/modular-tree-addon/) (newer code [here](https://github.com/steven-ray/modular_tree)) has blocks and should fix above, but IDK I don't see all stuff from [video](https://www.youtube.com/watch?v=UQx0eh8z-iM), and can't get simple square leaves.
3. [Tree-gen](https://github.com/friggog/tree-gen) and [video](https://www.youtube.com/watch?v=Zn_0I1cKaZE). I didn't like it, I don't see many options.

I've also gathered some jungle tree examples to know what I want to do in jungle and some general jungle knowledge.  
Nice [diagrams here](https://duckduckgo.com/?q=rainforest+layers+height+range&t=ftsa&atb=v395-1&iar=images&iax=images&ia=images) (e.g. [this one](https://3.bp.blogspot.com/-06hGv0ip5kA/V_-5LfiIwHI/AAAAAAAA4yY/nMD0A1KJpgMgzsCpMZb_ENALL5UhNl_ugCHM/s1600/emergent-layer-brazil-nut-tree-ref-tropical-rainforest.jpg)). Wow trees are huge in jungle: like up to 50m high for the biggest rare, main normal are up to 30m and those small palms and other are 15m. Heh, I can't even, it'll be a challenge doing all this it in SR3, as we ain't got even 1/3 of that and already my GPU touches limits. But I did recently reduce that [1.6M tris huge](https://sketchfab.com/3d-models/ceiba-pentandra-tree-9142b8f06e384f1c9554c9b10da79d2a) tree, to just 7k for trunk, will need more for leaves, but seems doable.

I also asked about manual LOD meshes. I find that auto LOD generator getting in the way. It always looses leaves first, makes them empty, etc. Meh I think even if more work it'd better to do LODs manualy, probably not possible. Either way it's done so in paid models.

- CryHam · 27 Jan 2024

Lol, looks like toilet paper. Anyway too high tris count.

I added more info about meshes. Also shows all materials and LODs now.

For Objects and Vegetation. Good info, since I don't remember what I used where.
😆

- CryHam · 9 Feb 2024

Okay I'll be using [improved-sapling-tree-generator](https://github.com/abpy/improved-sapling-tree-generator) from sapling_4 branch.

I'm quite happy with the features.

I'll save settings for each tree and .blend files for future, when I get any new trees.

I did only 1 tree, not finished yet.

Also it's a Blender add-on, where we need to export from so it's faster.

- CryHam · 15 Jun 2024

Okay finally some progress with jungle trees, just from ready models. IDK yet if it's too many triangles still for big tracks. But these look awesome. 3 new trees here now, triangles from 15k to 22k each, and 3 to 6 materials for each. Also I got refraction and depth color for waters but needs some work still.
😍

- Kris · 16 Jun 2024

Indeed that looks excellent. Pretty similar to SpeedTree I think.

- CryHam · 16 Jun 2024

Well this is something. The only problem is it's gonna need some decent optimizing to get it working for big tracks. It's a small track here.

- CryHam · 17 Jun 2024

Added some new old bushes. Feels amazing. But now it already drops below 60 Fps on this map.
I think this will be present on Tiny or Muddy and such small tracks.
😍

- Kris · 18 Jun 2024

Automatic billboard creation for distant trees like Unreal Engine does it? Not easy to do though with the correct lighting and all, I guess.

- CryHam · 18 Jun 2024

Yeah billboards for far trees would be cool, also for trees on horizons. We don't have them now, we had those in SR 2.x called impostors.
But my problem now is not even that. I need to get really good LOD meshes for each tree, manually made. Or just drop tree triangles even lower. They look still detailed.
Right now I got auto LODs for trees which still give like half of total triangles further (so e.g. will drop from 30 milion to 15) but look really ugly, since they just simply drop triangles, and look sparse. It's surely a challenge.

- CryHam · 2 Jul 2024

I'm adding many bushes, there are lots of models available. Should make forests better too.
Maybe even some will fit deserts.
👍

- CryHam · 9 Jul 2024

I got 3 new palms so far. Quite cool. They make a big difference. Our meshes got very old. Few bushes also fit deserts.
😍

- CryHam · 6 Aug 2024

I added bushes also to Forest and Mud sceneries.
We have now 6 new palms total. Present on Deserts, Island, Greece tracks.
I am currently dealing with Savannah trees etc. Replacing oldest meshes btw.
👏

- poVoq · 6 Aug 2024

Have you considered also changing the track texture and adding some track specific smaller plants? I think the last screenshot shows quite well how the track texture blends a bit too unrealistically with the green terrain texture.

I guess the original intention was to make it look like there is some vegetation on the track, but that isn't really how those tracks look in reality. They rather expose the soils below entirely, or are even covered with gravel manually placed there. There might still be some plants growing on the track here and there, but those look rather different to the near uniform green that currently blends through the track texture.

- CryHam · 7 Aug 2024

You mean road texture? Yes I don't really like it either. Also very old. I have some new textures in mind with gravel etc. Will need to replace someday. Lot of work of course.
I haven't even touched our ancient old mushrooms. I found many new, and want to replace on outer space sceneries. I think I'll do this earlier. OFC new jungle trees are priority, and replacing oldest meshes for complete CC list. Then maybe sounds, ugh.
👍

- CryHam · 11 Aug 2024

Allright.
Today is a celebration 🎉.
I've replaced all oldest trees and palms on all tracks.
The only left to do are shrooms big and small, and maybe old firs too.
The difference is huge in Jungle.
Also notable on Forest (got own new trees too), Islands, Moss, etc.
Jungle is a new thing now. I can't stop driving to look at new vegetation 😲.
You can see screens in our Tracks browser starting [from here](https://cryham.org/stuntrally/tracks?id=Jng1-Curly).
But this is great for high end GPUs. Lower will need reducing options as always.
🎉

- CryHam · 14 Aug 2024

Okay I did shrooms🍄. Lol, I mean I replaced oldest low poly, small texture mushrooms.
Changes can be seen on Aln, Uni tracks in our browser. E.g. [Aln here](https://stuntrally.tuxfamily.org/tracks3?id=Aln3-MushroomBeach), [or here](https://stuntrally.tuxfamily.org/tracks3?id=Aln1-AlienBase), [Uni here](https://stuntrally.tuxfamily.org/tracks3?id=Uni3-Cloud), [and here](https://stuntrally.tuxfamily.org/tracks3?id=Uni2-Swirl) etc.
