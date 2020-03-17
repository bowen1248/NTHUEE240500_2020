# Import module
#  csv -- fileIO operation
import csv

def max_temp(target, data):
   max = -9999999
   for row in data:
      if row["station_id"] == target:
         if row["TEMP"] > max:
            max = row["TEMP"]
   if max == -9999999:
      print(target, "None")
   else:
      print(target, max)
#=======================================
# Read cwb weather data
cwb_filename = '107061113.csv'
data = []
header = []

with open(cwb_filename) as csvfile:
   mycsv = csv.DictReader(csvfile)
   header = mycsv.fieldnames
   for row in mycsv:
      data.append(row)

#=======================================

# Analyze data depend on your group and store it to target_data like:
# Retrive all data points which station id is "C0X260" as a list:
# target_data = list(filter(lambda item: item['station_id'] == 'C0X260', data))

for row in data:
   if row["TEMP"] == -99 or row["TEMP"] == -999:
      del row

max_temp("C0A880", data)
max_temp("C0F9A0", data)
max_temp("C0G640", data)
max_temp("C0R190", data)
max_temp("C0X260", data)

# Print result

#print(data)

#========================================