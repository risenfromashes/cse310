import os
prefix = '1905005_'
for subdir, dirs, files in os.walk('.'):
    for file in files:
        if prefix == file[0:len(prefix)]:
            os.rename(os.path.join(subdir, file), os.path.join(subdir, file[len(prefix):]))
        else: 
            os.rename(os.path.join(subdir, file), os.path.join(subdir, prefix + file))

