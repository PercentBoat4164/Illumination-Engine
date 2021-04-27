import os
import platform
import tarfile

try:
    import gdown
except ImportError:
    print("Installing gdown...")
    os.system("python3 -m pip install gdown")
    print("Please run the program again.")
    exit()
gdown.download("https://drive.google.com/uc?id=1m4obHW53rdjeoW_RKvNURFT8cFA3x_jb", "Models.tar.xz", quiet=False)
print("Download finished\nExtracting...\nFrom: "+os.getcwd()+"Models.tar.xz\nTo: "+os.getcwd()+"/Models")
file = tarfile.open("Models.tar.xz", "r")
file.extractall("src/Models")
print("Cleaning up...\nRemoving: "+os.getcwd()+"/Models.tar.xz")
if platform.system() == "Linux":
    os.system("rm "+os.getcwd()+"/Models.tar.xz")
elif platform.system() == "Windows":
    os.system("del /f Models.tar.xz")
print("Done")
