import ast
import os
import sys
from io import BytesIO
from itertools import zip_longest
from pprint import pprint
from typing import BinaryIO
from slashr import SlashR
# noinspection PyUnresolvedReferences
from rm_lines import read_tree, CrdtSequence, CrdtSequenceItem, CrdtId
# noinspection PyUnresolvedReferences
from rm_lines.rmscene.text import expand_text_items, TextDocument

TEST_FILES_DIR = 'files'
PY_FILES_DIR = os.path.join("output", "text_expand")

for file in os.listdir(TEST_FILES_DIR):
    with open(os.path.join(TEST_FILES_DIR, file), "rb") as f:
        tree = read_tree(BytesIO(f.read()))
    if not tree.root_text:
        continue
    print(f"On file: {file[:-2]}", end='')
    with open(os.path.join(PY_FILES_DIR, file.replace('.rm', '.py')), 'rb') as f:
        lib_code = f.read()

        lib_data = eval(lib_code.decode('utf-8', errors='ignore'))
    char_items = CrdtSequence(expand_text_items(tree.root_text.items.sequence_items()))
    keys = list(char_items)
    # noinspection PyProtectedMember
    old_data = [
        char_items._items[key]
        for key in keys
    ]
    for i, (old, lib) in enumerate(zip_longest(old_data, lib_data, fillvalue=None)):
        if old != lib:
            print(f"\nMismatching on element {i}")
            print("Expected:")
            pprint(old)
            print('=' * 30)
            print("Lib value:")
            pprint(lib)
            exit()
    print('\r', end='')
print('Passed!')