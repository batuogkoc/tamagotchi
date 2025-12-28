import numpy as np
from PIL import Image
import argparse
import math
from pathlib import Path
import re

def to_snake_case(name: str) -> str:
	return "_".join(re.sub(r"[^a-zA-Z0-9]", "_", name).lower().strip().split("_"))

if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("filename")
	args = parser.parse_args()
	input_path = Path(args.filename)
	img = np.array(Image.open(input_path))

	img = img>img.mean()

	rows, cols = img.shape

	out_cols = math.ceil(rows/8)*8
	out_rows = rows

	out_img = np.zeros((out_rows, out_cols), dtype=np.bool_)

	out_img[:rows, :cols] = img

	sprite_name = to_snake_case(input_path.stem)
	with open(input_path.with_suffix(".h"), "w") as f:
		f.write(f"#define {sprite_name.upper()}_ROWS = {out_rows}\n")
		f.write(f"#define {sprite_name.upper()}_COLS = {out_cols}\n")
		f.write("static const unsigned char PROGMEM "+sprite_name+"[] = {\n")
		for row in range(out_rows):
			f.write("\t")
			for col in range(out_cols):
				pixel = out_img[row, col]
				if col%8 == 0:
					if col!=0:
						f.write(", ")
					f.write("0b")
				f.write(str(int(pixel)))
			if row!=out_rows-1:
				f.write(",\n")
			else:
				f.write(" };\n")
		
				
	

	