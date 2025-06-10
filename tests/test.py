from tests_base import *

total_time = 0
total_size_of_renderers = 0
total_size_of_trees = 0
for file in (files := os.listdir(files_folder)):
    svg_output_path = os.path.join(svg_output_folder, file.replace('.rm', '.svg'))
    png_output_path = os.path.join(png_output_folder, file.replace('.rm', '.png'))
    json_output_path = os.path.join(json_output_folder, file.replace('.rm', '.json'))
    md_output_path = os.path.join(md_output_folder, file.replace('.rm', '.md'))
    txt_output_path = os.path.join(txt_output_folder, file.replace('.rm', '.txt'))
    html_output_path = os.path.join(html_output_folder, file.replace('.rm', '.html'))

    print("Processing file:", file)
    begin = time.time()
    tree_id = lib.buildTree(os.path.join(files_folder, file).encode())
    total_time += (process_time := time.time() - begin)
    print(f"[{tree_id.decode()}] Read, time taken:", process_time)
    if not tree_id:
        raise Exception("Failed to build tree")

    begin = time.time()
    success = lib.convertToJsonFile(tree_id, json_output_path.encode())
    if not success:
        exit(-1)
    print(f"JSON (file) [{success}] Time taken:", time.time() - begin)
    begin = time.time()
    result = lib.convertToJson(tree_id)
    check_decode(result, 'JSON (raw)')
    print(f"JSON (raw) [{len(result)}] Time taken:", time.time() - begin)

    scene_info = lib.getSceneInfo(tree_id)
    if scene_info:
        print(f"Scene info: {scene_info.decode()}")
    paper_size = json.loads(scene_info.decode()).get('paper_size', (1404, 1872)) if scene_info else (1404, 1872)

    # Make a renderer

    begin = time.time()
    renderer_id = lib.makeRenderer(tree_id, 1, False)
    if not renderer_id:
        raise Exception("Failed to make renderer")

    total_time += (renderer_time := time.time() - begin)
    print(f"It took {renderer_time:.04f} to initialize the renderer")

    # Get the paragraphs
    paragraphs = lib.getParagraphs(renderer_id)
    if paragraphs:
        print(f"Paragraphs: {paragraphs.decode()}")
    # Get the paragraphs
    raw_layers = lib.getLayers(renderer_id)
    if raw_layers:
        str_layers = raw_layers.decode()
        print(f"Layers: {str_layers}")
        layers = json.loads(raw_layers.decode()) if str_layers else []
        for layer in layers:
            raw_size_tracker = lib.getSizeTracker(renderer_id, layer['groupId'].encode())
            if raw_size_tracker:
                print(f"Size tracker for layer {layer['groupId']}: {raw_size_tracker.decode()}")

    begin = time.time()
    success = lib.textToMdFile(renderer_id, md_output_path.encode())
    if not success:
        exit(-1)
    print(f"MD (file) [{success}] Time taken:", time.time() - begin)
    begin = time.time()
    result: bytes = lib.textToMd(renderer_id)
    check_decode(result, 'MD (raw)')
    print(f"MD (raw) [{len(result)}] Time taken:", time.time() - begin)

    begin = time.time()
    success = lib.textToTxtFile(renderer_id, txt_output_path.encode())
    if not success:
        exit(-1)
    print(f"TXT (file) [{success}] Time taken:", time.time() - begin)
    begin = time.time()
    result: bytes = lib.textToTxt(renderer_id)
    check_decode(result, 'MD (raw)')
    print(f"TXT (raw) [{len(result)}] Time taken:", time.time() - begin)

    begin = time.time()
    success = lib.textToHtmlFile(renderer_id, html_output_path.encode())
    if not success:
        exit(-1)
    print(f"HTML (file) [{success}] Time taken:", time.time() - begin)
    begin = time.time()
    result = lib.textToHtml(renderer_id)
    check_decode(result, 'HTML (raw)')
    print(f"HTML (raw) [{len(result)}] Time taken:", time.time() - begin)

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
        image.save(png_output_path, 'PNG')

    begin = time.time()
    size_of_renderer = lib.destroyRenderer(renderer_id)
    destroy_time = time.time() - begin
    print(f"Destroyed renderer [{size_of_renderer} bytes] in {destroy_time:.04f}")
    total_size_of_renderers += size_of_renderer

    begin = time.time()
    size_of_tree = lib.destroyTree(tree_id)
    destroy_time = time.time() - begin
    print(f"Destroyed tree [{size_of_tree} bytes] in {destroy_time:.04f}")
    total_size_of_trees += size_of_tree

    print("=" * 20)

print(f"All {len(files)} files processed in:", total_time)
print(f"Total size of all renderers: {total_size_of_renderers}")
print(f"Total size of all trees: {total_size_of_trees}")
