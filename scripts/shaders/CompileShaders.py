import os
import sys

def main():
    if len(sys.argv) < 2:
        print("No argument path! Exiting.")
        exit(1)

    if "VULKAN_SDK" not in os.environ:
         print("No VULKAN_SDK environment variable found! Exiting.")
         exit(1)

    fullPath = os.getcwd() + "/" + sys.argv[1] + "/"
    binPath = fullPath + "/bin/"
    command =  os.environ["VULKAN_SDK"] + "/Bin32/glslc.exe " + fullPath + "{} -o " + binPath + "{}"

    print("Looking for glsl files on folder: " + fullPath)
    glslList = list(filter(lambda name : name.find(".glsl") >= 0, os.listdir(fullPath)))
    if len(glslList) > 0 and not os.path.exists(binPath):
        print("Creating bin folder.")
        os.mkdir(binPath)

    for filename in glslList: 
        compiledFilename = filename[:filename.rfind(".")] + ".spv"
        print("Compiling {} into {}".format(filename, compiledFilename))
        os.system(command.format(filename, compiledFilename))

# Driver Code 
if __name__ == '__main__': 
    main()