from pygameextra import Rect

from tests_base import *

for dir in (files_draw_folder, files_folder):
    for file in (files := os.listdir(dir)):
        print(f"Processing file: {file[:-3]}")
        output_path = os.path.join(png_output_folder, file.replace('.rm', '.png'))

        tree_id = lib.buildTree(os.path.join(dir, file).encode())

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

        lib.getFrame(renderer_id, buffer, buffer_size * 4, 0, 0, *paper_size, *paper_size, True)
        raw_frame = bytes(buffer)

        if not raw_frame:
            print(f"Couldn't get frame [{len(raw_frame)}]")
            exit(-1)
        else:
            print(f"Got frame [{len(raw_frame)}]")
            image = Image.frombytes('RGBA', paper_size, raw_frame, 'raw', 'RGBA')
            image.save(output_path, 'PNG')

        rect = Rect(0, 0, *paper_size)
        combined = Image.new('RGB', (paper_size[0] * 3, paper_size[1]))
        print("Creating a scaling combined frame test...")
        for i, rect in enumerate((rect.scale_by(2, 2), rect, rect.scale_by(0.5, 0.5))):
            lib.getFrame(renderer_id, buffer, buffer_size * 4, *rect.topleft, *rect.size, *paper_size, True)
            raw_frame = bytes(buffer)
            image = Image.frombytes('RGBA', paper_size, raw_frame, 'raw', 'RGBA')
            combined.paste(image, (paper_size[0] * i, 0))
        for y in range(combined.height):
            combined.putpixel((paper_size[0] - 1, y), (150, 0, 0, 255))
            combined.putpixel((paper_size[0], y), (255, 0, 0, 255))
            combined.putpixel((paper_size[0] + 1, y), (150, 0, 0, 255))
            combined.putpixel((paper_size[0] * 2 - 1, y), (0, 0, 150, 255))
            combined.putpixel((paper_size[0] * 2, y), (0, 0, 255, 255))
            combined.putpixel((paper_size[0] * 2 + 1, y), (0, 0, 150, 255))

        combined.save(os.path.join(zoom_output_folder, file.replace('.rm', '.png')), 'PNG')

        lib.destroyRenderer(renderer_id)
        lib.destroyTree(tree_id)
