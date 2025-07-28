#!/usr/bin/python3
#!/usr/bin/env python3
import sys
import os
from PIL import Image
import PIL


#  Quick tool to crop screeshots from 1920x1080 to 1024x1024 for vehicle previews.
#  Info how to make them in cameras.xml at end.

#  params  -----
#dir = './_previews'
dir = '/home/ch/_sr/_sr3c/data/cars/_previews'

percent = 95   # save jpg %
size = 1024
left = 448
top = 28
#---------------
right = left + size
bottom = top + size

cnt = 0
#os.chdir(dir)


#  Main loop
#------------------------------------------------------------------------------------------
files = os.listdir(dir)
print(files)

for fname in files:
	fpath = os.path.join(dir, fname)
	if os.path.isfile(fpath):

		fspl = os.path.splitext(fname)
		fne = fspl[0]  # fname no ext
		ext = str.lower(fspl[1][1:])

		#  process jpg files with - in name,
		#  not already cropped with c
		if ext == 'jpg' and not 'c' in fne and '-' in fne:
			cnt += 1
			print('------- ' + str(cnt) + ' ---  ' + fname)

			img = Image.open(fpath)
			#fname = name[0] + '.jpg'
			fnew = os.path.join(dir, fne + 'c.jpg')  # new filename
			print(fnew)
			inew = img.crop((left, top, right, bottom))
			inew.save(fnew, quality=percent)

#  stats
print('---- End')
print('All: ' + str(cnt))
