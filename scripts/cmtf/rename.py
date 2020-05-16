import os
import sys

def main():   
    for count, filename in enumerate(os.listdir(sys.argv[1])): 
        if(filename.find(".hdr") >= 0):
            ext = filename[filename.rfind("."):]
            newname = filename[:filename.rfind("_")] + ext
            print(filename + " -> " + newname)
            os.rename(sys.argv[1] + "/" + filename, sys.argv[1] + "/" + newname) 

# Driver Code 
if __name__ == '__main__': 
    main()