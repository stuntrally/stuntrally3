#  INFO:
#  This script should get and build Stunt Rally 3 and its dependencies.
#  On Linux. In Release mode. Tested on Debian 12.
#
#  It will clone repos: ogre-next-deps, ogre-next, mygui-next, stuntrally3 and tracks3.
#  Note that this needs about 6 GB free space.
#
#  It is meant to be placed and run in empty dir, not inside git repo clone.
#  If it fails, next run will check and continue after done steps.
#  To force repeating a step, delete a build dir, or .git, etc. Details below.

#  user params  ----------
#  Could be changed by user
ogreBranch = 'v3-0'
myguiBranch = 'ogre3'

#-------------------------
import os
import sys

#  const paths  ----------
#  Shouldn't be changed
git = '.git'
build = 'build'
Ogre = 'Ogre'
ogre_next_deps = 'ogre-next-deps'
ogre_next = 'ogre-next'
Dependencies = 'Dependencies'
Release = 'Release'
mygui_next = 'mygui-next'
sr3 = 'sr3'
data = 'data'
tracks = 'tracks'
bin = 'bin'
binSRed = 'sr-editor3'
binSR = 'stuntrally3'
#-------------------------

print('===--- Start ')
root = os.getcwd()
print(root)


#  1  Ogre dir  -------------------------
print('===--- Step 1:  ' + Ogre)

if not os.path.exists(Ogre):
	print('==-- create dir: ' + Ogre)
	os.mkdir(Ogre)

os.chdir(Ogre)


#  2  ogre-next-deps  --------------------------------------------------
print('===--- Step 2:  ' + ogre_next_deps)

if not os.path.exists(ogre_next_deps):
	print('==-- create dir: ' + ogre_next_deps)
	os.mkdir(ogre_next_deps)
os.chdir(ogre_next_deps)

if not os.path.exists(git):
	print('===--- Step 2a:  git clone: ' + ogre_next_deps)
	ret = os.system('git clone --depth="1" --recurse-submodules --shallow-submodules https://github.com/OGRECave/ogre-next-deps .')
	if ret != 0:
		exit(ret)

if not os.path.exists(build):
	os.mkdir(build)
	os.chdir(build)
	print('===--- Step 2b:  building ' + ogre_next_deps)
	os.system('cmake  -G Ninja ..')
	os.system('ninja')
	#os.system('ninja install')  # ?-


#  3  ogre-next  --------------------------------------------------
os.chdir(root)
os.chdir(Ogre)
print('===--- Step 3:  ' + ogre_next)

if not os.path.exists(ogre_next):
	print('==-- create dir: ' + ogre_next)
	os.mkdir(ogre_next)
os.chdir(ogre_next)

if not os.path.exists(git):
	print('===--- Step 3a:  git clone: ' + ogre_next + ' ' + ogreBranch)
	ret = os.system('git clone --depth="1" --single-branch --branch '+ogreBranch+' https://github.com/OGRECave/ogre-next .')
	if ret != 0:
		exit(ret)

if not os.path.exists(Dependencies):
	print('===--- Step 3b:  link: ' + Dependencies)
	os.system('ln -s ../'+ogre_next_deps+'/'+build+'/ogredeps '+Dependencies)
	#os.system('ln -s ../ogre-next-deps/build/ogredeps Dependencies')

if not os.path.exists(build):
	os.mkdir(build)
	
os.chdir(build)
if not os.path.exists(Release):
	os.mkdir(Release)
	os.chdir(Release)
	print('===--- Step 3c:  build: ' + ogre_next + ' - ' + Release)

	ret = os.system('cmake -D OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS=1 '+
					'-D OGRE_CONFIG_THREAD_PROVIDER=0 -D OGRE_CONFIG_THREADS=0 '+
					#'-D OGRE_BUILD_COMPONENT_SCENE_FORMAT=1 '+
					#'-D OGRE_BUILD_SAMPLES2=1 -D OGRE_BUILD_TESTS=1 '+
					'-D OGRE_BUILD_SAMPLES2=1 -D OGRE_BUILD_TESTS=0 '+
					'-D CMAKE_BUILD_TYPE="Release"  -G Ninja ../..')
	if ret != 0:
		exit(ret)
	#print('cmake return: '+str(ret))

	ret = os.system('ninja')
	if ret != 0:
		exit(ret)


#  4  mygui-next  --------------------------------------------------
os.chdir(root)
print('===--- Step 4:  ' + mygui_next)

if not os.path.exists(mygui_next):
	print('==-- create dir: ' + mygui_next)
	os.mkdir(mygui_next)
os.chdir(mygui_next)

if not os.path.exists(git):
	print('===--- Step 4a:  git clone: ' + mygui_next + ' ' + ogreBranch)
	ret = os.system('git clone --depth="1" --single-branch --branch '+myguiBranch+' https://github.com/cryham/mygui-next .')
	if ret != 0:
		exit(ret)

if not os.path.exists(build):
	os.mkdir(build)
	
os.chdir(build)
if not os.path.exists(Release):
	os.mkdir(Release)
	os.chdir(Release)
	print('===--- Step 4b:  build: ' + mygui_next + ' - ' + Release)

	ret = os.system('cmake -D CMAKE_BUILD_TYPE="Release"  -G Ninja ../..')
	if ret != 0:
		exit(ret)

	ret = os.system('ninja')
	if ret != 0:
		exit(ret)


#  5  SR3  --------------------------------------------------
os.chdir(root)
print('===--- Step 5:  ' + sr3)

if not os.path.exists(sr3):
	print('==-- create dir: ' + sr3)
	os.mkdir(sr3)
os.chdir(sr3)

if not os.path.exists(git):
	print('===--- Step 5a:  git clone: ' + sr3)
	ret = os.system('git clone --depth="1" https://github.com/stuntrally/stuntrally3.git .')
	if ret != 0:
		exit(ret)

os.chdir(data)

if not os.path.exists(tracks):
	print('===--- Step 5b:  git clone: ' + sr3 + ' ' + tracks)
	ret = os.system('git clone --depth="1" https://github.com/stuntrally/tracks3.git '+tracks)
	if ret != 0:
		exit(ret)


#  6  SR3  build  --------------------------------------------------
os.chdir(root)
os.chdir(sr3)
print('===--- Step 6a:  cmake rename: ' + sr3)

#  rename CMake
if not os.path.exists('CMakeCI'):
	os.rename('CMake', 'CMakeCI')

if os.path.exists('CMakeManual'):
	os.rename('CMakeManual', 'CMake')

if not os.path.exists('CMakeListsCI.txt'):
	os.rename('CMakeLists.txt', 'CMakeListsCI.txt')

if os.path.exists('CMakeLists-Debian.txt'):
	os.rename('CMakeLists-Debian.txt', 'CMakeLists.txt')


if not os.path.exists(build):
	os.mkdir(build)
	
os.chdir(build)
if not os.path.exists(Release):
	os.mkdir(Release)
	os.chdir(Release)
	print('===--- Step 6b:  build: ' + sr3 + ' - ' + Release)

	ret = os.system('cmake -D CMAKE_BUILD_TYPE="Release"  -G Ninja ../..')
	if ret != 0:
		exit(ret)

	ret = os.system('ninja')
	if ret != 0:
		exit(ret)


#  7  SR3  test builds  --------------------------------------------------
print('===--- Step 7:  test builds: ' + sr3)

os.chdir(root)
path_bin = os.path.join(sr3, bin, Release)
pathSRed = os.path.join(path_bin, binSRed)
pathSR   = os.path.join(path_bin, binSR)

if not os.path.isfile(pathSRed):
	print('===--- Failed: not found editor binary: ' + pathSRed)
else:
	if not os.path.isfile(pathSR):
		print('===--- Failed: not found game binary: ' + pathSR)
	else:
		print('===--- Success!')
		print('change dir (cd) into: ' + path_bin)
		print('and start editor by: ./' + binSRed)
		print('   or start game by: ./' + binSR)

print('===---- End.')
