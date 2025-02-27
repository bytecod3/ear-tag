import matplotlib.pyplot as plt
import csv

# File containing data (modify with your filename)
filename = "../test-data/raw-data-1.csv"

# Lists to store data
x_values = []
y_values = []

# Read file and extract values
with open(filename, "r") as file:
    reader = csv.reader(file, delimiter='\n')
    for index,row in enumerate(reader):
        try:
            y_values.append(float(row[0]))
        except ValueError:
            print("Line {i} is corrupt!".format(i = index ))

# Plot the data
plt.figure(figsize=(8, 5))
plt.plot(y_values, linestyle="-", color="g", label="Data")

# Formatting
plt.xlabel("Time(m_sec)")
plt.ylabel("Acceleration magnitude (m/s^2)")
plt.title("Raw Magnitude of Acceleration")
plt.legend()
plt.grid()

# Show plot
plt.show()
