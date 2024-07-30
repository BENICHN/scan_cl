from numba import jit, prange
import numpy as np
import cv2
import sys

#########################################################################################################

def l2(c):
    return c @ c

def pos(x):
    return x if x > 0 else 0

def apply_gamma(src, gamma):
    """
    modifie le gamma d'une image
    # arguments
    * `src`: image source
    * `gamma`: nouveau gamma
    # retourne
    image avec gamma modifié
    """
    invGamma = 1 / gamma
    table = np.array([((i / 255) ** invGamma) * 255 for i in range(256)], dtype='b')
    return cv2.LUT(src, table).astype('uint8')

def largest_blocks(bw, n, axis, min_block_size, max_block_dist, min_connected_block_size):
    """
    calcule les `n` plus gros blocs réduits selon l'axe `axis` après éventuel recollement
    # arguments
    * `n` : nombre de blocs à retourner
    * `bw` image source au format bw
    * `axis` : axe de réduction
    * `min_block_size` : seuil de petits blocs
    * `max_block_dist` : seuil de distance entre blocs
    * `min_connected_block_size` : largeur minimale d'un bloc connecté
    # retourne
    les `n` blocs
    """
    cols = (np.sum(bw, axis=axis)>0).astype('uint8')
    blocks = cv2.connectedComponentsWithStats(cols)[2][1:,[1,3]]
    large_blocks = blocks[blocks[:,1] > min_block_size]
    large_connected_blocks = []
    for b in large_blocks:
        if len(large_connected_blocks) > 0 and large_connected_blocks[-1][1] + max_block_dist >= b[0]:
            large_connected_blocks[-1] = (large_connected_blocks[-1][0], b[0]+b[1])
        else:
            large_connected_blocks.append((b[0],b[0]+b[1]))
    big_connected_blocks = [ b for b in large_connected_blocks if b[1]-b[0] > min_connected_block_size ]
    big_connected_blocks.sort(key=lambda b: b[1]-b[0], reverse=True)
    largest_blocks = big_connected_blocks[0:n] if n != 0 else big_connected_blocks
    largest_blocks.sort(key=lambda b: b[0])
    return largest_blocks

@jit(cache=True)
def means(src, lab, n, stats, big_ccs):
    """
    calcule la moyenne des carrés des couleurs de chaque composante connexe d'une image
    # arguments
    * `src` : image
    * `lab` : labels des composantes connexes
    * `n` : nombre de composantes connexes
    * `stats` : statistiques des composantes connexes
    * `big_ccs` indices des grosses composantes connexes
    """
    res = np.zeros(n)
    for i in prange(lab.shape[0]):
        for j in prange(lab.shape[1]):
            cc = lab[i,j]
            if cc > 0: res[cc] += src[i,j] ** 2
    return res[big_ccs] / stats[big_ccs,4]

twos = 2**np.arange(8)[::-1]
@jit(cache=True)
def bwtobits(bw):
    a = bw.size
    bw1 = bw.reshape(a)
    n = int(np.ceil(a/8))
    res = np.zeros(n, dtype='uint8')
    for i in prange(n):
        byt = bw1[8*i:8*(i+1)]
        s = byt.size
        j = np.sum(byt * twos[:s])
        res[i]=j
    return res
@jit(cache=True)
def bwtobitsaligned(bw):
    h, w = bw.shape
    nw = int(np.ceil(w/8))
    n = h*nw
    res = np.zeros(n, dtype='uint8')
    for i in prange(h):
        row = bw[i]
        for j in prange(nw):
            byt = row[8*j:8*(j+1)]
            s = byt.size
            k = np.sum(byt*twos[:s])
            res[i*nw+j]=k
    return res

def process_img(gray, clr, op):
    color_mode = op['color_mode']
    bw = (gray < 255).astype('uint8')
    h, w = bw.shape
    # On détermine les plages x du contenu
    x_blocks = largest_blocks(bw, op['num_pages'], 0, op['min_block_size'], op['max_block_dist'], op['min_connected_block_size'])
    if not len(x_blocks) == op['num_pages']: raise Exception("pas assez de pages dans l'image")
    for x_start, x_end in x_blocks:
        bw1 = bw[:,x_start:x_end]
        
        # On détermine la plage y du contenu
        
        y_start, y_end = largest_blocks(bw1, 1, axis=1, max_block_dist=h, min_connected_block_size=0, min_block_size=op['min_block_size'])[0]
        bw2 = bw1[y_start:y_end]
        
        # On calcule le rectangle et la matrice de rotation
        
        nzc = cv2.findNonZero(bw2)
        box = cv2.minAreaRect(points=nzc)
        box_vertices = cv2.boxPoints(box=box)
        angle = -box[2]
        if (angle < -45):
            angle = 90 + angle
        if angle == 0: angle = 0.02 # pour que les images non tournées paraissent comme les tournées
        M = cv2.getRotationMatrix2D(center=(w/2, h/2), angle=-angle, scale=1.0)
        
        # Rotation de l'image selon le rectangle
        
        rotated = gray.copy()
        cv2.warpAffine(src=gray, M=M, dsize=(w, h), dst=rotated, flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REPLICATE)
        if color_mode != 'no':
            rotated_clr = clr.copy()
            cv2.warpAffine(src=clr, M=M, dsize=(w, h), dst=rotated_clr, flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_REPLICATE)
        
        # Clippage de l'image selon le rectangle

        box_vertices_sorted = np.array(sorted(box_vertices, key=lambda c: l2(c)))
        box_vertices_matrix = np.vstack((box_vertices_sorted.transpose(), np.ones((1,4))))
        box_vertices_rotated = ((M @ box_vertices_matrix).transpose()).astype('i')
        offset = np.array([x_start, y_start])
        tl = box_vertices_rotated[0]+offset
        br = box_vertices_rotated[3]+offset

        crop_overflow=op['crop_overflow']
        cropped = rotated[pos(tl[1]-crop_overflow):br[1]+1+crop_overflow, pos(tl[0]-crop_overflow):br[0]+1+crop_overflow]
        if color_mode != 'no':
            cropped_clr = rotated_clr[pos(tl[1]-crop_overflow):br[1]+1+crop_overflow, pos(tl[0]-crop_overflow):br[0]+1+crop_overflow]
        
        # Flippage

        if op['flip']:
            flipped = np.flip(np.flip(cropped, axis=0), axis=1)
            if color_mode != 'no': flipped_clr = np.flip(np.flip(cropped_clr, axis=0), axis=1)
        else:
            flipped = cropped
            if color_mode != 'no': flipped_clr = cropped_clr
            
        bw3 = (flipped < op['white_threshold']).astype('uint8')

        # Ajustement des couleurs

        if color_mode != 'no':
            flipped_gray = flipped_clr if color_mode == 'gray' else cv2.cvtColor(flipped_clr, cv2.COLOR_BGR2GRAY)
            gmask = (flipped_gray < op['white_color_threshold']).astype('uint8')
            whitened_clr = (255 - (255 - flipped_clr) * (gmask if color_mode == 'gray' else gmask[:,:,np.newaxis])).astype('uint8')
            adjusted_clr = apply_gamma(whitened_clr, op['gamma'])
    
        # Effacement des petits blocs

        n, lab, stats, cent = cv2.connectedComponentsWithStats(bw3)
        small_ccs_filter = stats[:,4] < op['small_image_blocks_area']
        small_ccs = np.nonzero(small_ccs_filter)[0]
        small_ccs_mask = (~np.isin(lab, small_ccs)).astype('uint8')
        bw4 = bw3 * small_ccs_mask
        
        # Effacement des gros blocs
    
        big_ccs = np.nonzero(~small_ccs_filter)[0][1:]
        big_ccs_means = means(255-flipped, lab, n, stats, big_ccs)
        big_light_ccs = big_ccs[big_ccs_means < op['max_big_cc_color_mean']].astype('i')
        big_light_ccs_mask = (np.isin(lab, big_light_ccs)).astype('uint8')

        bw4_blurred = (cv2.blur(bw4 * 255, ksize=op['blur_size']) > 0).astype('uint8')
        n_blurred, lab_blurred, stats_blurred, cent_blurred = cv2.connectedComponentsWithStats(bw4_blurred)
        big_ccs_blurred = np.nonzero(stats_blurred[:,4] < op['max_blurred_cc_area'])
        big_ccs_blurred_mask = (np.isin(lab_blurred, big_ccs_blurred)).astype('uint8')
        big_ccs_mask = big_ccs_blurred_mask * bw4

        big_ccs_total_mask = ((big_light_ccs_mask + big_ccs_mask) > 0).astype('uint8')
        bigs = np.any(big_ccs_total_mask)
        if bigs:
            n_bigs, lab_bigs, stats_bigs, cent_bigs = cv2.connectedComponentsWithStats(big_ccs_total_mask)
            return bw4, adjusted_clr, (n_bigs, lab_bigs, stats_bigs, cent_bigs)
        return bw4, adjusted_clr, None            
        
##             newbmask = big_ccs_total_mask.copy()
##             print('/!\\ big blocks detected')
##             bgsmask = (cv2.blur(big_ccs_total_mask * 255, ksize=(50,50)) == 0).astype('uint8')
##             diff = (1 - nz1) * 255 / (1+bgsmask)
##             nbg, labbg, statsbg, centbg = cv2.connectedComponentsWithStats(1-bgsmask)
## 
##             def onclose(inv):
##                 nonlocal newbmask
##                 if inv:
##                     newbmask = (newbmask < 0).astype('uint8')
##                 else:
##                     newbmask = (newbmask > 0).astype('uint8')
## 
##             def invcc(iv, c, p):
##                 nonlocal newbmask
##                 if c != 0:
##                     labc = lab * big_ccs_total_mask * (labbg == c).astype('i')
##                     labcnz = np.nonzero(labc)
##                     if p is None:
##                         ccs = np.unique(labc)
##                     else:
##                         m = np.argmin((labcnz[0] - p[0]) ** 2 + (labcnz[1] - p[1]) ** 2)
##                         ccs = [lab[labcnz[0][m], labcnz[1][m]]]
##                     newbmask[np.isin(lab, ccs)] *= -1
##                     diff[labbg == c] /= 2
##                     iv.update_data(diff.astype('uint8'))
## 
##             def onclick(iv, x, y):
##                 c = labbg[y,x]
##                 invcc(iv, c, (x, y))
## 
##             def onselection(iv, tl, br):
##                 xs = sorted((tl.x(), br.x()))
##                 ys = sorted((tl.y(), br.y()))
##                 cs = np.unique(labbg[pos(ys[0]):ys[1]+1, pos(xs[0]):xs[1]+1])
##                 for c in cs:
##                     invcc(iv, c, None)
## 
##             QImageViewer(diff.astype('uint8'), QImage.Format.Format_Grayscale8, 1, False, 0, onclick, onclose, onselection).showMaximized()
##             big_ccs_total_mask = newbmask
##         nz2 = nz1 * (1 - big_ccs_total_mask)
##         nz2f = nz * smask * (1 - big_ccs_total_mask)
##         nz2y, nz2x = np.nonzero(nz2f)
##         nt = np.min(nz2y)
##         nb = np.max(nz2y)
##         nl = np.min(nz2x)
##         nr = np.max(nz2x)
##         nz2c = nz2[pos(nt-corr):nb+1+corr,pos(nl-corr):nr+1+corr]
##         cleaned = 1 - nz2c
##         if color: selectedclrc = selectedclr[pos(nt-corr):nb+1+corr,pos(nl-corr):nr+1+corr]
    
#########################################################################################################

source = sys.argv[1]
destDir = sys.argv[2]
destName = sys.argv[3]

# f = open('log.log', 'w')
# f.write(source)
# f.write(destDir)
# f.write(destName)

gray = cv2.imread(source, cv2.IMREAD_GRAYSCALE)
bwf, clrf, bigs = process_img(gray, gray, {
    'num_pages': 1,
    'max_block_dist': 60,
    'min_connected_block_size': 250,
    'min_block_size': 10,
    'crop_overflow': 50,
    'flip': True,
    'white_threshold': 234, # seuil de blanc (<255)
    'small_image_blocks_area': 20, # taille des petits blocs d'image
    'color_mode': 'gray', # taille des petits blocs d'image
    'white_color_threshold': 240, # seuil de blanc pour les couleurs (<=255)
    'gamma': 0.5,
    'max_big_cc_color_mean': 15000,
    'blur_size': (110,12),
    'max_blurred_cc_area': 4000
})
cv2.imwrite(f'{destDir}/{destName}.pbm', bwf)
# cv2.imwrite("test_clr.pgm", clrf)

# h, w = bwf.shape
# sys.stdout.buffer.write(w.to_bytes(4,sys.byteorder))
# sys.stdout.buffer.write(h.to_bytes(4,sys.byteorder))
# 
# b = bwtobitsaligned(bwf).tobytes()
# sys.stdout.buffer.write(b)
# 
# if bigs:
#     n_bigs, lab_bigs, stats_bigs, cent_bigs = bigs
#     sys.stdout.buffer.write(n_bigs.to_bytes(4,sys.byteorder))
#     # sys.stdout.buffer.write(lab_bigs.astype('int32').tobytes())
#     sys.stdout.buffer.write(stats_bigs[:,0:4].astype('int32').tobytes())
#     sys.stdout.buffer.write(cent_bigs.astype('int32').tobytes())
# else:
#     sys.stdout.buffer.write(bytes(0,0,0,0))