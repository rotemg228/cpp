import requests
from os.path import exists
if exists("webhookData.txt") and exists("webhookUrl.txt"):
    webhookData = {"content": open("webhookData.txt", "r").read()}
    webhookUrl = open("webhookUrl.txt", "r").read()
    req = requests.post(webhookUrl, data=webhookData)
    open("discord_res.txt", "w").write(str(req.status_code))
