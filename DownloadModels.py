import os
import platform
try:
    import gdown
except ImportError:
    print("Installing gdown...")
    os.system("pip3 install gdown")
    print("Please run the program again.")
    exit()
try:
    import tarfile
except ImportError:
    print("Installing tarfile...")
    os.system("pip3 install tarfile")
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
    os.system("del /f "+os.getcwd()+"/Models.tar.xz")
print("Done")
