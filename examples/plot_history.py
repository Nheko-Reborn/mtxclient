import pandas as pd

# Script to plot the file from memberstats

tsv_data = pd.read_csv('history.tsv', sep='\t')
tsv_data = tsv_data.sort_values(by=['timestamp'])
tsv_data["timestamp"] = pd.to_datetime(tsv_data["timestamp"], unit = 'ms')

count = 0
def inc():
    global count
    count += 1
    return count
def dec():
    global count
    count -= 1
    return count

tsv_data['count'] = tsv_data.apply((lambda x: inc() if x['joined'] == 'j' else dec()), axis=1)

print(tsv_data.head(20))
print(tsv_data.tail(20))

p = tsv_data.plot.line(x='timestamp', y='count')
p.get_figure().savefig('nheko_hist.png')

