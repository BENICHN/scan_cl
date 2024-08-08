import numpy as np
import cv2
import sys

f = open('log.log', 'w')
f.write(str(sys.argv))
f.write('\n')

try:
    destDir = sys.argv[1]
    destName = sys.argv[2]
    colorMode = sys.argv[3]
    maskPath = sys.argv[4]

    bwPath = destDir + '/' + destName + '.pbm'
    clrPath = destDir + '/' + destName + ('.pgm' if colorMode == 'PT_GRAY' else '.ppm')

    bw = (cv2.imread(bwPath, cv2.IMREAD_GRAYSCALE) > 0).astype('uint8')
    clr = cv2.imread(clrPath, cv2.IMREAD_GRAYSCALE if colorMode == 'PT_GRAY' else cv2.IMREAD_COLOR)
    mask = cv2.imread(maskPath, cv2.IMREAD_GRAYSCALE) > 0

    newBw = bw * ~mask
    newClr = clr * mask

    cv2.imwrite(bwPath, newBw)
    cv2.imwrite(clrPath, newClr)
except Exception as e:
    f.write("ex : ")
    f.write(str(e))
    raise
