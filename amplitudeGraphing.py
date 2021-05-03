import matplotlib.pyplot as plt
import csv

x = []
y = []
y2 = []
count = 0

with open('Amplitudes.csv','r') as csvfile:
    points = csv.reader(csvfile, delimiter = ',')
    for row in points:
        if (count > 100000) and (count < 200000) and (count%5000 == 0):
            x.append(row[0])
            y.append(float(row[1]))
            y2.append(float(row[2]))
            print("here")
        count = count + 1
        print(count)
    
fig, axs = plt.subplots(2)
axs[0].plot(x,y,'tab:green')
axs[0].set(xlabel='Sample Number', ylabel = 'Amplitude')
axs[0].set_title('Unfiltered Amplitude x Sample')
axs[1].plot(x,y2, 'tab:orange')
axs[1].set(xlabel='Sample Number', ylabel='Amplitude')
axs[1].set_title('Filtered Amplitude x Sample')
plt.show()