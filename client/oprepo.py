#-------------------------------------------------------------------------------
# Name:        OPrepo Client
# Author:      Ethan
# Created:     23/01/2017
# Copyright:   (c) Ethan 2017
#-------------------------------------------------------------------------------
import os
import requests
import getpass
import time
from datetime import datetime, timedelta, date
import json
import base64
import sys
import zipfile

baseURL = "http://159.203.47.121"   #Actual server address
#baseURL = "http://localhost:3000"  #For local debugging
year = "2017" #Year for repository reference only allows up/down to said repo
loginURL = baseURL + "/client/login"
createNewRepoURL = baseURL + "/r/" + year + "/new"
logoutURL = baseURL + "/logout"
downloadURL = baseURL + "/r/" + year + "/download"


#wrapper for asking y/n questions WITH defaults
def askQuestion(question, default="yes"):
    valid = {"yes": True, "y": True,
             "no": False, "n": False}
    if default is None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)
    while True:
        sys.stdout.write(question + prompt)
        choice = raw_input().lower()
        if default is not None and choice == '':
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' " "(or 'y' or 'n').\n")


#prints the help page
def helpPage():
    print ""
    print "----------------------------------------help----------------------------------------"
    print "help\t\t\tLoads this menu"
    print "generate\t\tCreate local info file to save directories"
    print "upload\t\t\tUploads files in the current OR saved directory"
    print "csdrup\t\t\tUploads files from a specified directory"
    print "download\t\tDownloads the most recent upload to the current OR saved directory"
    print "downloadzip\t\tDownloads the most recent upload to the current OR saved directory in .zip format"
    print "csdrdn\t\t\tDownloads the most recent upload to the specified directory"
    print "csdrdnzip\t\tDownloads the most recent upload to the specified directory in .zip format"
    print "exit\t\t\tExits the program"
    print ""


#some wack code I found that ensures we aren't reading a binary file...
#doesn't cover fringe cases but it works pretty decently for what I need
def is_text_file(filepathname):
    textchars = bytearray([7,8,9,10,12,13,27]) + bytearray(range(0x20, 0x7f)) + bytearray(range(0x80, 0x100))
    is_binary_string = lambda bytes: bool(bytes.translate(None, textchars))
    if is_binary_string(open(filepathname, 'rb').read(1024)):
       return False
    else:
       return True


#simplified login function that returns a session if login is correct
#and a null object and an error message if the login fails
def generateNewSession():
    r = requests.Session()
    print "Please Login:"
    un = raw_input('Username: ')
    up = getpass.getpass()
    loginDetails = {
        'oprusername':un,
        'oprpassword':up
        }
    resp = r.post(loginURL, data=loginDetails).text
    j = json.loads(resp)
    print "\n"
    if j['response'] == "OK":
        return {'session':r, 'error':'', 'user':j['user']}
    else:
        return {'session':None, 'error':j['error']}


#upload files in current directory (relative to the script)
#OR from the save file (if it exists)
def uploadFromCurrentDirectory():
    print "File Uploader!"
    req = generateNewSession()
    if req['session'] is None:
        print "Error: " + req['error']
    else:
        print "Logged in as " + req['user']
        r = req['session']
        n = datetime.now()
        rm = raw_input('Commit Message: ')
        newPostDetails = {
            'oprdate':n.strftime("%m/%d/%Y %H:%M:%S"),
            'oprmessage': rm
        }
        resp = r.post(createNewRepoURL, data=newPostDetails).text
        j = json.loads(resp)
        if j['response'] == "OK":
            if datFileExists() and askQuestion("Use saved upload location?"):
                uploadPath = readDatFile()['up']
                repoID = j['newid']
                uploadFileToRepoURL = baseURL + "/r/" + year + "/" + repoID + "/upload"
                fileList = []
                fileIgnoreList = []
                print "Searching directory..."
                for dirname, dirnames, filenames in os.walk(uploadPath):
                    for filename in filenames:
                        if filename != "client.py":
                            if is_text_file(uploadPath + "\\" + filename):
                                fileList.append(filename)
                            else:
                                fileIgnoreList.append(filename)
                    if '.git' in dirnames:
                        dirnames.remove('.git')
                print "Found " + str(len(fileList)) + " files."
                if askQuestion("Upload them now? "):
                    pc = 0
                    ma = len(fileList)
                    uploadFailList = []
                    for f in fileList:
                        q = open(uploadPath + "\\" + f, "r+")
                        c = q.read()
                        q.close()
                        fileData = {
                            'data':base64.b64encode(c.encode('ascii')),
                            'name':f
                        }
                        print "Uploading: " + f
                        upResp = r.post(uploadFileToRepoURL, data=fileData).text
                        jsonResp = json.loads(upResp)
                        if jsonResp['response'] == 'OK':
                            pc+=1
                            print str(pc) + "/" + str(ma) + " Upload Successful!"
                        else:
                            uploadFailList.append({
                                "name":f,
                                "error":jsonResp['error']
                                })
                            print str(pc) + "/" + str(ma) + " Upload Failed with error: " + jsonResp['error']

                    print "Uploaded " + str(pc) + "/" + str(ma) + " files successfully."
                    if len(uploadFailList) > 0:
                        print "Failed Files: "
                        for fa in uploadFailList:
                            print "\t" + fa.name + " :: " + fa.error
                else:
                    print "Skipping upload..."

                print "Logging out..."
                r.get(logoutURL)
                print "Logged out"
            else:
                repoID = j['newid']
                uploadFileToRepoURL = baseURL + "/r/" + year + "/" + repoID + "/upload"
                fileList = []
                fileIgnoreList = []
                print "Searching directory..."
                for dirname, dirnames, filenames in os.walk('.'):
                    for filename in filenames:
                        if filename != "client.py":
                            if is_text_file(filename):
                                fileList.append(filename)
                            else:
                                fileIgnoreList.append(filename)
                    if '.git' in dirnames:
                        dirnames.remove('.git')
                print "Ignoring the following " + str(len(fileIgnoreList)) + " files: "
                for fil in fileIgnoreList:
                    print fil
                print "Found " + str(len(fileList)) + " files."
                if askQuestion("Upload them now? "):
                    pc = 0
                    ma = len(fileList)
                    uploadFailList = []
                    for f in fileList:
                        q = open(f, "r+")
                        c = q.read()
                        q.close()
                        fileData = {
                            'data':base64.b64encode(c.encode('ascii')),
                            'name':f
                        }
                        print "Uploading: " + f
                        upResp = r.post(uploadFileToRepoURL, data=fileData).text
                        jsonResp = json.loads(upResp)
                        if jsonResp['response'] == 'OK':
                            pc+=1
                            print str(pc) + "/" + str(ma) + " Upload Successful!"
                        else:
                            uploadFailList.append({
                                "name":f,
                                "error":jsonResp['error']
                                })
                            print str(pc) + "/" + str(ma) + " Upload Failed with error: " + jsonResp['error']

                    print "Uploaded " + str(pc) + "/" + str(ma) + " files successfully."
                    if len(uploadFailList) > 0:
                        print "Failed Files: "
                        for fa in uploadFailList:
                            print "\t" + fa.name + " :: " + fa.error
                else:
                    print "Skipping upload..."
                print "Logging out..."
                r.get(logoutURL)
                print "Logged out"
    print ""

def uploadFromOtherDirectory(customdDirectory):
    print "File Uploader!"
    req = generateNewSession()
    if req['session'] is None:
        print "Error: " + req['error']
    else:
        print "Logged in as " + req['user']
        r = req['session']
        rm = raw_input('Commit Message: ')
        n = datetime.now()
        newPostDetails = {
            'oprdate':n.strftime("%m/%d/%Y %H:%M:%S"),
            'oprmessage': rm
        }
        resp = r.post(createNewRepoURL, data=newPostDetails).text
        j = json.loads(resp)
        if j['response'] == "OK":
            repoID = j['newid']
            uploadFileToRepoURL = baseURL + "/r/" + year + "/" + repoID + "/upload"
            fileList = []
            fileIgnoreList = []
            print "Searching directory..."
            for dirname, dirnames, filenames in os.walk(customdDirectory):
                for filename in filenames:
                    if filename != "client.py":
                        if is_text_file(customdDirectory + "\\" + filename):
                            fileList.append(filename)
                        else:
                            fileIgnoreList.append(filename)
                if '.git' in dirnames:
                    dirnames.remove('.git')
            print "Found " + str(len(fileList)) + " files."
            if askQuestion("Upload them now? "):
                pc = 0
                ma = len(fileList)
                uploadFailList = []
                for f in fileList:
                    q = open(customdDirectory + "\\" + f, "r+")
                    c = q.read()
                    q.close()
                    fileData = {
                        'data':base64.b64encode(c.encode('ascii')),
                        'name':f
                    }
                    print "Uploading: " + f
                    upResp = r.post(uploadFileToRepoURL, data=fileData).text
                    jsonResp = json.loads(upResp)
                    if jsonResp['response'] == 'OK':
                        pc+=1
                        print str(pc) + "/" + str(ma) + " Upload Successful!"
                    else:
                        uploadFailList.append({
                            "name":f,
                            "error":jsonResp['error']
                            })
                        print str(pc) + "/" + str(ma) + " Upload Failed with error: " + jsonResp['error']

                print "Uploaded " + str(pc) + "/" + str(ma) + " files successfully."
                if len(uploadFailList) > 0:
                    print "Failed Files: "
                    for fa in uploadFailList:
                        print "\t" + fa.name + " :: " + fa.error
            else:
                print "Skipping upload..."

            print "Logging out..."
            r.get(logoutURL)
            print "Logged out"
    print ""

def downloadToCurrentDirectoryZip():
    print "File Downloader!"
    req = generateNewSession()
    if req['session'] is None:
        print "Error: " + req['error']
    else:
        print "Logged in as " + req['user']
        r = req['session']
        print "Loading data from server..."
        resp = r.get(downloadURL).text
        j = json.loads(resp)
        print "Data loaded with response: " + j['response']
        if j['response'] == "OK":
            if datFileExists() and askQuestion("Use saved download location?"):
                fdata = j['zipfile']['data']
                fname = j['zipfile']['name']
                n = datetime.strptime(j['date'], "%m/%d/%Y %H:%M:%S")
                fdirname = n.strftime("%Y_%m_%d_%H_%M_%S")
                fdir = readDatFile()['down'] + "\\" + fdirname + "\\"
                if not os.path.exists(fdir):
                    print "directory does not exist"
                    print "creating new directory..."
                    os.mkdir(fdir)
                    print "directory created successfully"
                else:
                    print "directory already exists"
                if askQuestion("Use recomended file name \"" + fname + "\"?"):
                    print "Writing File \"" + fname + "\"..."
                    fdir = fdir + "\\" + fname
                    with open(fdir, "wb") as fh:
                        fh.write(base64.decodestring(fdata))
                    print "File written successfully!"
                else:
                    newFname = raw_input("New file name (with .zip extension): ")
                    fdir = fdir + "\\" + newFname
                    print "Writing File \"" + newFname + "\"..."
                    with open(fdir, "wb") as fh:
                        fh.write(base64.decodestring(fdata))
                    print "File written successfully!"
                print "Logging out..."
                r.get(logoutURL)
                print "Logged out"
            else:
                fdata = j['zipfile']['data']
                fname = j['zipfile']['name']
                n = datetime.strptime(j['date'], "%m/%d/%Y %H:%M:%S")
                fdirname = n.strftime("%Y_%m_%d_%H_%M_%S")
                fdir = os.getcwd() + "\\" + fdirname + "\\"
                if not os.path.exists(fdir):
                    print "directory does not exist"
                    print "creating new directory..."
                    os.mkdir(fdir)
                    print "directory created successfully"
                else:
                    print "directory already exists"
                if askQuestion("Use recomended file name \"" + fname + "\"?"):
                    print "Writing File \"" + fname + "\"..."
                    fdir = fdir + "\\" + fname
                    with open(fdir, "wb") as fh:
                        fh.write(base64.decodestring(fdata))
                    print "File written successfully!"
                else:
                    newFname = raw_input("New file name (with .zip extension): ")
                    fdir = fdir + "\\" + newFname
                    print "Writing File \"" + newFname + "\"..."
                    with open(fdir, "wb") as fh:
                        fh.write(base64.decodestring(fdata))
                    print "File written successfully!"
                print "Logging out..."
                r.get(logoutURL)
                print "Logged out"
    print ""

def downloadToCurrentDirectory():
    print "File Downloader!"
    req = generateNewSession()
    if req['session'] is None:
        print "Error: " + req['error']
    else:
        print "Logged in as " + req['user']
        r = req['session']
        print "Loading data from server..."
        resp = r.get(downloadURL).text
        j = json.loads(resp)
        print "Data loaded with response: " + j['response']
        if j['response'] == "OK":
            if datFileExists() and askQuestion("Use saved download location?"):
                fdata = j['zipfile']['data']
                fname = j['random_string'] + ".zip"
                fdirzip = readDatFile()['down'] + "\\" + fname
                print "Creating zip file..."
                with open(fdirzip, "wb") as fh:
                    fh.write(base64.decodestring(fdata))
                fh.close()
                print "Zip file created successfully"
                print "checking for directory..."
                n = datetime.strptime(j['date'], "%m/%d/%Y %H:%M:%S")
                fdirname = n.strftime("%Y_%m_%d_%H_%M_%S")
                fdir = readDatFile()['down'] + "\\" + fdirname
                if not os.path.exists(fdir):
                    print "directory does not exist"
                    print "creating new directory..."
                    os.mkdir(fdir)
                    print "directory created successfully"
                else:
                    print "directory already exists"
                print "extracting files..."
                zfile = zipfile.ZipFile(fdirzip)
                zfile.extractall(fdir)
                zfile.close()
                print "files extracted."
                print "deleting zip file..."
                os.remove(fdirzip)
                print "zip file deleted successfully"
                print "process completed successfully"
                print "Logging out..."
                r.get(logoutURL)
                print "Logged out"
            else:
                fdata = j['zipfile']['data']
                fname = j['random_string'] + ".zip"
                print "Creating zip file..."
                with open(fname, "wb") as fh:
                    fh.write(base64.decodestring(fdata))
                fh.close()
                print "Zip file created successfully"
                print "checking for directory..."
                n = datetime.strptime(j['date'], "%m/%d/%Y %H:%M:%S")
                fdirname = n.strftime("%Y_%m_%d_%H_%M_%S")
                fdir = os.getcwd() + "\\" + fdirname
                if not os.path.exists(fdir):
                    print "directory does not exist"
                    print "creating new directory..."
                    os.mkdir(fdir)
                    print "directory created successfully"
                else:
                    print "directory already exists"
                print "extracting files..."
                zfile = zipfile.ZipFile(fname)
                zfile.extractall(fdir)
                zfile.close()
                print "files extracted."
                print "deleting zip file..."
                os.remove(fname)
                print "zip file deleted successfully"
                print "process completed successfully"
                print "Logging out..."
                r.get(logoutURL)
                print "Logged out"
    print ""

def downloadToCustomDirectory(customdDirectory):
    print "File Downloader!"
    req = generateNewSession()
    if req['session'] is None:
        print "Error: " + req['error']
    else:
        print "Logged in as " + req['user']
        r = req['session']
        print "Loading data from server..."
        resp = r.get(downloadURL).text
        j = json.loads(resp)
        print "Data loaded with response: " + j['response']
        if j['response'] == "OK":
            fdata = j['zipfile']['data']
            fname = j['random_string'] + ".zip"
            fdirzip = customdDirectory + "\\" + fname
            print "Creating zip file..."
            with open(fdirzip, "wb") as fh:
                fh.write(base64.decodestring(fdata))
            fh.close()
            print "Zip file created successfully"
            print "checking for directory..."
            n = datetime.strptime(j['date'], "%m/%d/%Y %H:%M:%S")
            fdirname = n.strftime("%Y_%m_%d_%H_%M_%S")
            fdir = customdDirectory + "/" + fdirname
            if not os.path.exists(fdir):
                print "directory does not exist"
                print "creating new directory..."
                os.mkdir(fdir)
                print "directory created successfully"
            else:
                print "directory already exists"
            print "extracting files..."
            zfile = zipfile.ZipFile(fdirzip)
            zfile.extractall(fdir)
            zfile.close()
            print "files extracted."
            print "deleting zip file..."
            os.remove(fdirzip)
            print "zip file deleted successfully"
            print "process completed successfully"
            print "Logging out..."
            r.get(logoutURL)
            print "Logged out"
    print ""

def downloadToCustomDirectoryZip(customdDirectory):
    print "Zip File Downloader!"
    req = generateNewSession()
    if req['session'] is None:
        print "Error: " + req['error']
    else:
        print "Logged in as " + req['user']
        r = req['session']
        print "Loading data from server..."
        resp = r.get(downloadURL).text
        j = json.loads(resp)
        print "Data loaded with response: " + j['response'] + " " + j['error']
        if j['response'] == "OK":
            fdata = j['zipfile']['data']
            fname = j['zipfile']['name']
            n = datetime.strptime(j['date'], "%m/%d/%Y %H:%M:%S")
            fdirname = n.strftime("%Y_%m_%d_%H_%M_%S")
            fdir = customdDirectory + "\\" + fdirname + "\\"
            if not os.path.exists(fdir):
                print "directory does not exist"
                print "creating new directory..."
                os.mkdir(fdir)
                print "directory created successfully"
            else:
                print "directory already exists"
            if askQuestion("Use recomended file name \"" + fname + "\"?"):
                print "Writing File \"" + fname + "\"..."
                fdir = fdir + "\\" + fname
                with open(fdir, "wb") as fh:
                    fh.write(base64.decodestring(fdata))
                print "File written successfully!"
            else:
                newFname = raw_input("New file name (with .zip extension): ")
                fdir = fdir + "\\" + newFname
                print "Writing File \"" + newFname + "\"..."
                with open(fdir, "wb") as fh:
                    fh.write(base64.decodestring(fdata))
                print "File written successfully!"
            print "Logging out..."
            r.get(logoutURL)
            print "Logged out"
    print ""


def generateDatFile():
    print "Local Data file generator"
    if datFileExists():
        print "Data file already exists..."
        if askQuestion("Would you like to overwrite?", "no"):
            createDatFileRAW()
        else:
            print "Not overwriting"
    else:
        createDatFileRAW()

def createDatFileRAW():
    print "fill in the fields below for improved use"
    defDownDir = raw_input("Default download directory path: ")
    defUpDir = raw_input("Default upload directory path: ")
    print "generating file..."
    if not os.path.exists(defDownDir):
        os.mkdir(defDownDir)
    if not os.path.exists(defUpDir):
        os.mkdir(defUpDir)
    datInfo = "{\"up\":\"" + defUpDir + "\",\"down\":\"" + defDownDir + "\"}"
    datInfo = datInfo.replace("\\", "\\\\")
    datFile = open('client_info.dat', 'w')
    datFile.write(datInfo)
    datFile.close()
    print "file generated successfully"

def datFileExists():
    return os.path.isfile('client_info.dat')

def readDatFile():
    datFile = open('client_info.dat', 'r')
    dat = str(datFile.read())
    datFile.close()
    jdat = json.loads(dat)
    return jdat

def main():
    running = True
    if not datFileExists():
        print "It looks like you don't have a save file"
        print "It's recomended that you generate a save file to speed up file transfer\n"
        if askQuestion("Would you like to generate one now?"):
            generateDatFile()
        else:
            print "OK"
            print "Launching into program...\n\n"
    while running:
        i = raw_input("~oprepo$ ")
        co = [
            "exit",
            "help",
            "upload",
            "downloadzip",
            "download",
            "csdrup",
            "csdrdn",
            "csdrdnzip",
            "generate",
        ]
        if i ==  co[0] or i == co[0].upper() or i == co[0].title():
            running = False
        elif i ==  co[1] or i == co[1].upper() or i == co[1].title():
            helpPage()
        elif i ==  co[2] or i == co[2].upper() or i == co[2].title():
            uploadFromCurrentDirectory()
        elif i ==  co[3] or i == co[3].upper() or i == co[3].title():
            downloadToCurrentDirectoryZip()
        elif i ==  co[4] or i == co[4].upper() or i == co[4].title():
            downloadToCurrentDirectory()
        elif i ==  co[5] or i == co[5].upper() or i == co[5].title():
            customdir = raw_input("Enter directory path: ")
            if os.path.isdir(customdir):
                uploadFromOtherDirectory(customdir)
            else:
                print "Invalid Directory"
        elif i ==  co[6] or i == co[6].upper() or i == co[6].title():
            customdir = raw_input("Enter directory path: ")
            if os.path.isdir(customdir):
                downloadToCustomDirectory(customdir)
            else:
                print "Invalid Directory"
        elif i ==  co[7] or i == co[7].upper() or i == co[7].title():
            customdir = raw_input("Enter directory path: ")
            if os.path.isdir(customdir):
                downloadToCustomDirectoryZip(customdir)
            else:
                print "Invalid Directory"
        elif i ==  co[8] or i == co[8].upper() or i == co[8].title():
            generateDatFile()
        elif i == "":
            pass
        else:
            print "Unknown command.\nType 'help' for more information"
    pass

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        raise
    except Exception as e:
        print e
        pass