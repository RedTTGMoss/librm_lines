import mmap
import os
import ctypes

lib = ctypes.CDLL("../cmake-build-debug/librm_lines.so")

# Function signature: (int, size_t, int) -> size_t
lib.convertToSvg.argtypes = [ctypes.c_int, ctypes.c_size_t, ctypes.c_int]
lib.convertToSvg.restype = ctypes.c_size_t


@ctypes.CFUNCTYPE(None, ctypes.c_char_p)
def python_logger(msg):
    print(msg.decode())


lib.setLogger(python_logger)

output_path = "output.svg"
with open(output_path, "wb") as f:
    pass

# Open both files
with open("test.rm","r+b") as fin, open(output_path, "r+b") as fout:
    # Memory-map the input file
    input_mm = mmap.mmap(fin.fileno(), 0, access=mmap.ACCESS_READ)

    # Call the shared library (it will expand the output file)
    written_size = lib.convertToSvg(fin.fileno(), len(input_mm), fout.fileno())

    # Re-open and remap output since its size changed
    fout.close()
    with open(output_path, "r+b") as fout:
        output_mm = mmap.mmap(fout.fileno(), 0, access=mmap.ACCESS_READ)

        # Read back the result
        result_svg = output_mm.read(written_size).decode()

# Print the result
print("Received SVG:", result_svg)
