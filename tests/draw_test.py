from tests_base import *

for file in (files := os.listdir(files_folder)):
    print(f"Processing file: {file[:-3]}")
    output_path = os.path.join(output_folder, file.replace('.rm', '.png'))

    tree_id = lib.buildTree(os.path.join(files_folder, file).encode())

    if not tree_id:
        raise Exception("Failed to build tree")

    renderer_id = lib.makeRenderer(tree_id, 1, False)

    if not renderer_id:
        raise Exception("Failed to make renderer")

    scene_info = lib.getSceneInfo(tree_id)
    if scene_info:
        print(f"Scene info: {scene_info.decode()}")
    paper_size = json.loads(scene_info.decode())['paperSize'] or (1404, 1872) if scene_info else (1404, 1872)
    if scene_info:
        print(f"Paper size: {paper_size}")

    buffer_size = paper_size[0] * paper_size[1]
    buffer = (ctypes.c_uint32 * buffer_size)()

    lib.getFrame(renderer_id, buffer, buffer_size * 4, 0, 0, *paper_size, 1)
    raw_frame = bytes(buffer)

    if not raw_frame:
        print(f"Couldn't get frame [{len(raw_frame)}]")
        exit(-1)
    else:
        print(f"Got frame [{len(raw_frame)}]")
        image = Image.frombytes('RGBA', paper_size, raw_frame, 'raw', 'RGBA')
        image.save(output_path, 'PNG')

    lib.destroyRenderer(renderer_id)
    lib.destroyTree(tree_id)
