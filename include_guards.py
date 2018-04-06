#!/usr/bin/env python3

import os
import glob

# recursively list all the files with a specific extension
def files(path, ext):
    output = glob.glob(path + '/**/*.' + ext, recursive=True)
    return output
def label(header):
    basename = os.path.basename(header)
    fileName = os.path.splitext(basename)[0]

    label = list()
    label.extend(list("OPEN3D"))

    upper = list()
    prvUpper = False
    first = True
    for c in list(fileName):
        if c.isupper():
            upper.append(c)
            continue
        else:
            if 1 == len(upper):
                label.append('_')
                first = False
                label.extend(upper)
                upper = list()
                label.append(c.upper())
            elif 1 < len(upper):
                label.append('_')
                first = False
                label.extend(upper[:-1])
                label.append('_')
                label.append(upper[-1])
                upper = list()
                label.append(c.upper())
            else:
                if first:
                    label.append('_')
                    first = False
                label.append(c.upper())

    if 0 < len(upper):
        label.append('_')
        label.extend(upper)

    label.append('_H')

    output = "".join(label)

    return output

paths = [
    "/home/dpetre/Open3D/issue_278/src/Core",
    "/home/dpetre/Open3D/issue_278/src/Experimental",
    "/home/dpetre/Open3D/issue_278/src/IO",
    "/home/dpetre/Open3D/issue_278/src/Python",
    "/home/dpetre/Open3D/issue_278/src/Test",
    "/home/dpetre/Open3D/issue_278/src/Tools",
    "/home/dpetre/Open3D/issue_278/src/Visualization"
]

for path in paths:
    headers = files(path, 'h')

    for header in headers:
        basename = os.path.basename(header)
        guard = label(basename)
        print("%-50s%s" % (guard, basename))
