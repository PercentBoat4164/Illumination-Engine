import os
try:
    import gdown
except ImportError:
    print("Installing gdown...")
    os.system("pip3 install gdown")
    print("Please run the program again.")
try:
    import tarfile
except ImportError:
    print("Installing tarfile...")
    os.system("pip3 install tarfile")
    print("Please run the program again.")
gdown.download("https://drive.google.com/uc?id=1m4obHW53rdjeoW_RKvNURFT8cFA3x_jb", "Models.tar.xz", quiet=False)
print("Download finished\nExtracting...\nFrom: "+os.getcwd()+"/Models.tar.xz\nTo: "+os.getcwd()+"/Models")
file = tarfile.open("Models.tar.xz", "r")
file.extractall("Models")
print("Done")
