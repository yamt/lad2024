import csv
import io
import sys
import xml.etree.ElementTree as ET

tmx = sys.argv[1]
tree = ET.parse(tmx)
root = tree.getroot()
csvstr = root.findtext("./layer/data")
f = io.StringIO(csvstr)
csvreader = csv.reader(f)
f.close

tiles = "_WBXLDRUPA"

print("    {")
print("        .data = (const uint8_t[]){")
indent = " " * 12
for row in csvreader:
    if not row:
        continue
    l = ""
    for c in row:
        if c == "":
            continue
        l += f"{tiles[int(c)]}, "
    print(f"{indent}{l}END,")
print(f"{indent}END,")
print("        },")
print("    },")
