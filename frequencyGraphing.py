import matplotlib.pyplot as plt
import csv

x = []
y = []

with open('FreqOutput.csv','r') as csvfile:
    points = csv.reader(csvfile, delimiter = ',')

    for row in points:
            x.append(row[0])
            y.append(float(row[1]))

plt.plot(x,y, label = 'Frequency')
plt.xlabel('Sample Number')
plt.ylabel('Frequency Value')
plt.title('Frequency x Samples')
plt.legend()
plt.show()
    