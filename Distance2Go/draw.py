import numpy as np
import matplotlib.pyplot as plt
import time

# 通过for-in循环逐行读取
with open('outputData.log', mode='r') as f:
    i=0
    times = []
    values = []
    for line in f:
        if len(line.split())>0:
            times.append(i)
            i = i+1
            print (i)
            #print (line)
            #print (line.split())
            #value = line.split()[1] * 100
            value = line
            values.append(value)
            #print(value, end='')
print(times)
print(values)
        
    
# 画图
plt.figure(figsize = (7,4)) 
plt.plot(times,values)
plt.savefig("outputImageByPython.png")
