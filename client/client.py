#-------------------------------------------------------------------------------
# Name:        OPrepo Client
# Author:      Ethan
# Created:     23/01/2017
# Copyright:   (c) Ethan 2017
#-------------------------------------------------------------------------------
import os
import requests
import getpass
from datetime import datetime, timedelta, date
import json
import base64

baseURL = "http://159.203.47.121"
#baseURL = "http://localhost"
year = "2017"
loginURL = baseURL + "/ulogin"
createNewRepoURL = baseURL + "/r/" + year + "/new"

def main():
    r = requests.Session()
    print "File Uploader!"
    print "Please Login:"
    un = raw_input('Username: ')
    up = getpass.getpass()
    rm = raw_input('Commit Message: ')
    loginDetails = {
        'oprusername':un,
        'oprpassword':up
    }
    n = datetime.now()
    newPostDetails = {
        'oprdate':n.strftime("%m/%d/%Y %H:%M:%S"),
        'oprmessage': rm
    }
    r.post(loginURL, data=loginDetails)
    resp = r.post(createNewRepoURL, data=newPostDetails).text
    j = json.loads(resp)
    if j['response'] == "OK":
        repoID = j['newid']
        uploadFileToRepoURL = baseURL + "/r/" + year + "/" + repoID + "/upload"
        fileList = []
        for dirname, dirnames, filenames in os.walk('.'):
            for filename in filenames:
                if filename != "client.py":
                    fileList.append(filename)
            if '.git' in dirnames:
                dirnames.remove('.git')
        pc = 0

        for f in fileList:
            q = open(f, "r+")
            c = q.read()
            q.close()
            fileData = {
                'data':base64.b64encode(c.encode('ascii')),
                'name':f
            }
            r.post(uploadFileToRepoURL, data=fileData)
    pass

if __name__ == '__main__':
    main()
