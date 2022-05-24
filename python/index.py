import cv2
import numpy as np

vid = cv2.VideoCapture(2, cv2.CAP_DSHOW)

width = 96; height = 64
dim = 6
r = 2; g = 1; b = 0
l = 0.3
a = 0.5

filter = np.full((height, width, 3), 255, np.uint8)
filteredImg = np.zeros((height, width, 3), np.uint8)
integralImage = np.zeros((height, width, 3), np.uint8)

lut = np.zeros((255), np.uint8)
for i in range(len(lut)):
    x =  i / 255.0
    x = ((np.sign(2*x - 1) * (abs(2*x - 1)**a) + 1) / 2).astype(np.float16)
    x = 255.0 - x * 255.0
    lut[i] = x.astype(np.uint8)
    print(lut[i])


while(True):
    ret, img = vid.read()
    img = cv2.resize(img, (width, height))
    
    for j in range(len(img)):
        for i in range(len(img[j])):
            for k in range(len(img[j][i])):
                imgElement = img[j][i][k].astype(np.float16)
                integralImage[j][i][k] = ((integralImage[j][i][k] + (l * imgElement)) / (1 + l))
                #average = (integralImage[j][i][0] + integralImage[j][i][1] + integralImage[j][i][2]) / 3
                #deviation = 1 - ((integralImage[j][i][k] - average))
                maxR = max(integralImage[j][i][0], integralImage[j][i][1], integralImage[j][i][2])
                minR = min(integralImage[j][i][0], integralImage[j][i][1], integralImage[j][i][2])
                s = (maxR - minR) / maxR if maxR != minR else 0

                #derivative = prevFilter[j][i][k] - filter[j][i][k]
                
                filter[j][i][k] = (filter[j][i][k] + s * lut[(integralImage[j][i][k] * s).astype(np.uint8)]) / (1+s)
                #filter[j][i][k] = (filter[j][i][k] + (1 - s) * 255) /(2-s)
                filteredImg[j][i][k] = img[j][i][k].astype(np.float16) * filter[j][i][k].astype(np.float16) / 255.0

    filteredImgD = cv2.resize(filteredImg, (width * dim, height * dim))
    imgD = cv2.resize(img, (width * dim, height * dim))
    integralImageD = cv2.resize(integralImage, (width * dim, height * dim))
    filterD = cv2.resize(filter, (width * dim, height * dim))

    vis1 = np.concatenate((imgD, integralImageD), axis=1)
    vis2 = np.concatenate((filterD, filteredImgD), axis=1)
    visTot = np.concatenate((vis1, vis2), axis=0)
    cv2.imshow('frame', visTot)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
    prevFilter = filter
  
vid.release()
cv2.destroyAllWindows()