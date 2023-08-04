#!/usr/bin/python3
#!/usr/bin/env python3
import sys
import os

#  script to convert  .mesh files
#  from v1 SR 2.7 to v2 SR 3.0  with lods

#  setup dirs  -----
work_dir = '/home/ch/_sr/og3/Ogre/ogre-next/build/Release/bin/'
tool = './OgreMeshTool'
media = '/home/ch/_sr/og3/_sr/Media/'
recursive = False

#start_dir = os.getcwd()
#recursive = True

# start_dir = media + 'trees2'
# args = ' -v2 -e -t -ts 4 -O puqs -l 4 -d 200 -p 20 '  #  objs
# args = ' -v2 -e -t -ts 4 -O puqs -l 6 -d 200 -p 15 '  #  trees, rocks

#  configs for Media subdirs
#------------------------------
#  objects3/  new obstacles, hq
start_dir = media + 'objects3/'
args = ' -v2 -e -t -ts 4 -O puqs -l 2 -d 400 -p 12 '  # 3 lods

#  rocks/  hex*
#start_dir = media + 'rockshex/'
#args = ' -v2 -e -t -ts 4 -O puqs -l 2 -d 400 -p 12 '  # 3 lods

#  objects0/  0 A.D. buildings
#start_dir = media + 'objects0/'
#args = ' -v2 -e -t -ts 4 -O puqs '  # no lods +
# args = ' -v2 -e -t -ts 4 -O puqs -l 2 -d 600 -p 5 '  # 3 lods bad-

#  objects2/  satelite, hq
#start_dir = media + 'objects2/'
#args = ' -v2 -e -t -ts 4 -O puqs -l 3 -d 600 -p 15 '  # 4 lods far

#  objectsC/  alien,city,skyscraper
#start_dir = media + 'objectsC/'
#args = ' -v2 -e -t -ts 4 -O puqs '  # no lods +
# args = ' -v2 -e -t -ts 4 -O puqs -l 2 -d 600 -p 10 '  # 3 lods far bad-
#------------------------------

cnt = 0
err = 0
#new_ext = '+.mesh'
os.chdir(work_dir)


#  Process 1 file ext
#------------------------------------------------------------------------------------------
def process_file_ext(dir, fname):
	global cnt
	global err
	fpath = os.path.join(dir, fname)
	if not os.path.isfile(fpath):
		return
	
	fspl = os.path.splitext(fname)
	fne = fspl[0]  # fname no ext
	ext = str.lower(fspl[1][1:])
	
	#size = os.path.getsize(fpath)/1000  # KiB
	#print(fname+' '+ext+' '+str(size))

	if ext == 'mesh':
		cnt += 1
		#new_fpath = os.path.join(dir, fne + new_ext)
		print('------- ' + str(cnt) + ' ---  ' + fname)
		#cmd = tool + ' -v2 -l 4 -d 200 -p 20 ' + fname  # old
		cmd = tool + args + dir +'/'+ fname  # new opt
		print(cmd)
		ret = os.system(cmd)
		if ret != 0:
			err += 1
		#exit(0)


#  Main loop
#------------------------------------------------------------------------------------------
if not recursive:
	files = os.listdir(start_dir)

	for fname in files:
		process_file_ext(start_dir, fname)
else:
	for dir, subdirs, files in os.walk(start_dir):
		if dir.find('/.') != -1:  # skip hidden
			continue

		for fname in files:
			process_file_ext(dir, fname)

#  stats
print('---- End')
print('All: ' + str(cnt) + '  errors: ' + str(err))
