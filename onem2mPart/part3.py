from onem2m import *
import matplotlib.pyplot as plotter



data = []
init_time = []

data_dict = {}

for i in range(60):
    data_dict[i] = 0;

uri_cse = "http://127.0.0.1:8080/~/in-cse/in-name"
ae = "BabyMonitor"
cnt = "node1"

uri_ae = uri_cse + "/" + ae
uri_cnt = uri_ae + "/" + cnt

resp = get_data(uri_ae + "/node1?rcn=4")
print(resp)

for dat in resp["m2m:cnt"]["m2m:cin"]:
    data.append(float(dat["con"]))
    init_time.append(dat["ct"][-4:-2] + ":" + dat["ct"][-2:])
    
    data_dict[int(dat["con"])] += 1

print(data)
print(init_time)
plotter.scatter(list(data_dict.keys()), list(data_dict.values()), color="red");

plotter.show()
