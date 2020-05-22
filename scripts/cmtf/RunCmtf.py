import os
import sys

def rename(dir):   
    for filename in os.listdir(dir): 
        if(filename.find(".hdr") >= 0):
            ext = filename[filename.rfind("."):]
            newname = filename[:filename.rfind("_")] + ext
            print(filename + " -> " + newname)
            os.rename(dir + "/" + filename, dir + "/" + newname) 

def main():
    if len(sys.argv) < 2:
        print("No argument path to cmtf! Exiting.")
        exit(1)

    if len(sys.argv) < 3:
        print("No argument input skybox file! Exiting.")
        exit(1)

    inputImagePath = sys.argv[2].replace("\\", "/")
    inputDir = inputImagePath[:inputImagePath.rfind("/")]


    print("Converting to face list...")
    skyboxDir = inputDir + "/skybox"
    if not os.path.exists(skyboxDir):
        os.mkdir(skyboxDir)

    command = ("{}/cmftRelease.exe "
               "--input {} "
               "--filter none "
               "--outputNum 1 "
               "--output0 {}/skybox "
               "--output0params hdr,rgbe,facelist")
    os.system(command.format(sys.argv[1], inputImagePath, skyboxDir))


    print("Doing irradiance...")
    irradianceDir = inputDir + "/irradiance"
    if not os.path.exists(irradianceDir):
        os.mkdir(irradianceDir)

    command = ("{}/cmftRelease.exe "
               "--input {} "
               "--filter irradiance "
               "--dstFaceSize 512  "
               "--outputNum 1 "
               "--output0 {}/skybox "
               "--output0params hdr,rgbe,facelist")
    os.system(command.format(sys.argv[1], inputImagePath, irradianceDir))


    print("Doing radiance...")
    radianceDir = inputDir + "/radiance"
    if not os.path.exists(radianceDir):
        os.mkdir(radianceDir)

    command = ("{}/cmftRelease.exe "
               "--input {} "
               "--filter radiance "
               "--srcFaceSize 512  "
               "--dstFaceSize 512  "
               "-excludeBase false "
               "-mipCount 5 "
               "-glossScale 10 "
               "-glossBias 3 "
               "-lightingModel blinnbrdf "
               "-numCpuProcessingThreads 4 "
               "-useOpenCL true "
               "-clVendor anyGpuVendor "
               "-deviceType gpu "
               "-deviceIndex 0 "
               "-inputGammaNumerator 1.0 "
               "-inputGammaDenominator 1.0 "
               "-outputGammaNumerator 1.0 "
               "-outputGammaDenominator 1.0 "
               "-generateMipChain false "
               "--outputNum 1 "
               "--output0 {}/skybox "
               "--output0params hdr,rgbe,facelist")
    os.system(command.format(sys.argv[1], inputImagePath, radianceDir))

    print("Renaming files...")
    rename(radianceDir)

# Driver Code 
if __name__ == '__main__': 
    main()