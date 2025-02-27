import matplotlib.pyplot as plt
import csv

# File containing data (modify with your filename)
filename = "../test-data/raw-filtered-data-1.csv"

# Lists to store data
raw_values = []
filtered_values = []

# Read file and extract values
with open(filename, "r") as file:
    reader = csv.reader(file, delimiter=',')
    for index,row in enumerate(reader):
        try:
            raw_values.append(float(row[0]))
            filtered_values.append(float(row[1]))
        except ValueError:
            print("Line {i} is corrupt!".format(i = index ))

# Plot the data
plt.figure(figsize=(8, 5))
plt.plot(raw_values, linestyle="-", color="g", label="Raw |ACC|")
plt.plot(filtered_values, linestyle="-", color="r", label="Filtered |ACC|")

# Formatting
plt.xlabel("Time(m_sec)")
plt.ylabel("Acceleration magnitude (m/s^2)")
plt.title("Filtered+Raw Magnitude of Acceleration")
plt.legend()
plt.grid()

# Show plot
plt.show()
