@echo off

if "%~1"=="" (
    @echo Please supply the name of the input skybox file.
    goto end
)

if not exist %1 (
    @echo The input file %1% does not exist.
    goto end
)

@echo Input file: %1
mkdir skybox
@echo Converting to face list...
cmftRelease.exe ^
         --input %1 ^
         --filter none ^
         --outputNum 1 ^
         --output0 ./skybox/skybox ^
         --output0params hdr,rgbe,facelist

mkdir irradiance
@echo.
@echo Doing irradiance...
cmftRelease.exe ^
         --input %1 ^
         --filter irradiance ^
         --dstFaceSize 512 ^
         --outputNum 1 ^
         --output0 ./irradiance/skybox ^
         --output0params hdr,rgbe,facelist

mkdir radiance
@echo.
@echo Doing Radiance...
cmftRelease.exe ^
         --input %1 ^
         --filter radiance ^
         --srcFaceSize 512 ^
         --dstFaceSize 512 ^
         --excludeBase false ^
         --mipCount 5 ^
         --glossScale 10 ^
         --glossBias 3 ^
         --lightingModel blinnbrdf ^
         --numCpuProcessingThreads 4 ^
         --useOpenCL true ^
         --clVendor anyGpuVendor ^
         --deviceType gpu ^
         --deviceIndex 0 ^
         --inputGammaNumerator 1.0 ^
         --inputGammaDenominator 1.0 ^
         --outputGammaNumerator 1.0 ^
         --outputGammaDenominator 1.0 ^
         --generateMipChain false ^
         --outputNum 1 ^
         --output0 ./radiance/skybox ^
         --output0params hdr,rgbe,facelist

@echo.
@echo Renaming Radiance files...
python rename.py ./radiance

:end
pause