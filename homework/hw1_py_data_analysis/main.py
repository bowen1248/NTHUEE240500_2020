# Import module
#  csv -- fileIO operation
import csv

def max_temp(target, data, result):
   max_val = -9999999
   for row in data:
      if row["station_id"] == target:
         if float(row["TEMP"]) > max_val:
            max_val = float(row["TEMP"])
   if max_val == -9999999 or max_val == -99 or max_val == -999:
      result.append([target, "None"])
   else:
      result.append([target, max_val])
#=======================================
# Read cwb weather data
cwb_filename = 'sample_input.csv'
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

result = []
max_temp("C0A880", data, result)
max_temp("C0F9A0", data, result)
max_temp("C0G640", data, result)
max_temp("C0R190", data, result)
max_temp("C0X260", data, result)
print(result)
