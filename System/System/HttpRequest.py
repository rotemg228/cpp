import requests
from os.path import exists
Url = open("httprequrl.txt", "r").read()
Method = open("httpreqmet.txt", "r").read()
outputFile = open("httpreqres.txt", "w")
if Method == "GET":
    outputFile.write(requests.get(Url).text)
elif exists("httpreqheaders.txt") and exists("httpreqdata.txt"):
    headers = open("httpreqheaders.txt", "r")
    data = open("httpreqdata.txt", "r")
    res = requests.post(Url,headers=headers,data=data)
    outputFile.write(res.status_code)
