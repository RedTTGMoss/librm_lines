const folders = [
    "/tests/draw_files/",
    "/tests/files/",
    "/tests/color_files/"
];
const TILE_SIZE = 256;
let tileQueue = Promise.resolve();
let rmLines;
let viewer;
let tile_source;

function queueTile(fn) {
    return fn();
}

function alphaToWhite(imageData) {
    const d = imageData.data;

    for (let i = 0; i < d.length; i += 4) {
        const a = d[i + 3] / 255;

        d[i] = d[i] * a + 255 * (1 - a);
        d[i + 1] = d[i + 1] * a + 255 * (1 - a);
        d[i + 2] = d[i + 2] * a + 255 * (1 - a);

        d[i + 3] = 255;
    }

    return imageData;
}

function createTileSource(file) {
    const state = {
        tree: null,
        renderer: null,
        paper_size: [1404, 1872],
        bounds: [0, 0, 1404, 1872],
        offset: [0, 0]
    };

    async function init() {
        logInfo("Initializing viewer for file", prettifyUrl(file));
        state.tree = await linesBuildTree(file);
        if (!state.tree) {
            return false;
        }

        // Read scene info
        const sceneInfo = linesGetSceneInfo(state.tree);
        if (sceneInfo && sceneInfo.paper_size) {
            state.paper_size = sceneInfo.paper_size;
        }

        state.renderer = linesMakeRenderer(state.tree, 1, false);
        if (!state.renderer) {
            return false;
        }
        state.bounds = getMaxBounds(state.renderer, state.paper_size)
        if (state.bounds[0] < 0 || state.bounds[1] < 0) {
            state.offset = [-state.bounds[0], -state.bounds[1]]
            state.bounds[0] = 0;
            state.bounds[1] = 0;
            state.bounds[2] = state.bounds[2] + state.offset[0];
            state.bounds[3] = state.bounds[3] + state.offset[1];
        }
        const scale = Math.pow(this.maxLevel + 4, 2);
        this.width = state.bounds[2] * scale;
        this.height = state.bounds[3] * scale;
        return true;
    }

    function destroy() {
        if (state.renderer) {
            linesDestroyRenderer(state.renderer);
        }
        if (state.tree) {
            linesDestroyTree(state.tree);
        }
    }

    const tileSize = TILE_SIZE;

    return {
        width: 1000000,
        height: 1000000,
        tileSize: tileSize,
        tileOverlap: 0,
        minLevel: 0,
        maxLevel: 6,

        init,
        destroy,


        getTileUrl: function (level, x, y) {
            return `dynamic://${level}/${x}/${y}`;
        },

        getTileHashKey(level, x, y) {
            return `${level}_${x}_${y}`;
        },

        getTilePostData(level, x, y) {
            return {
                level: level,
                dx: x,//level ? Math.floor(x / (level + 1)) : x,
                dy: y//level ? Math.floor(y / (level + 1)) : y
            };
        },

        getTileSize(level, x, y) {
            return {
                width: tileSize,
                height: tileSize
            }
        },
        getTileFrame(level, x, y) {
            const rect = [
                x * tileSize,
                y * tileSize,
                tileSize,
                tileSize
            ]
            const scale = level ? 1 / Math.pow(2, level) : 1;
            // const scale = level + 1;

            return {
                x: state.offset[0] + rect[0] * scale,
                y: state.offset[1] + rect[1] * scale,
                width: rect[2] * scale,
                height: rect[3] * scale
            };
        },

        async downloadTileStart(context) {
            let size = this.getTileSize(context.postData.level, context.postData.dx, context.postData.dy);
            let frame = this.getTileFrame(context.postData.level, context.postData.dx, context.postData.dy);
            const canvas = document.createElement("canvas");
            const ctx = canvas.getContext("2d");

            frame.x = Math.floor(frame.x);
            frame.y = Math.floor(frame.y);
            frame.width = Math.floor(frame.width);
            frame.height = Math.floor(frame.height);
            frame.r = frame.x + frame.width;
            frame.b = frame.y + frame.height;

            if (size.width < 1 || size.height < 1 ||
                (frame.x < state.bounds[0] || frame.x > state.bounds[2] || frame.y < state.bounds[1] || frame.y > state.bounds[3]) &&
                (frame.r < state.bounds[0] || frame.r > state.bounds[2] || frame.b < state.bounds[1] || frame.y > state.bounds[3])
            ) {
                canvas.width = 1;
                canvas.height = 1;
                context.finish(ctx, null, "context2d");
                return;
            } else {
                canvas.width = size.width;
                canvas.height = size.height;
            }
            ctx.fillStyle = "red";
            ctx.fillRect(0, 0, canvas.width, canvas.height);

            await renderTile(state.renderer, ctx, frame, size);

            // state.renderer.renderTile(ctx, level, x, y);
            context.finish(ctx, null, "context2d");
        }
    }
}

async function renderTile(renderer_id, ctx, frame, size) {
    const image = await linesGetFrame(
        renderer_id,
        frame.x,
        frame.y,
        frame.width,
        frame.height,
        size.width,
        size.height,
        false
    )
    ctx.putImageData(alphaToWhite(image), 0, 0);
}


async function getFiles(folder) {
    const res = await fetch(folder);

    if (!res.ok) return [];

    const html = await res.text();

    const doc = new DOMParser().parseFromString(html, "text/html");

    return [...doc.querySelectorAll("a")]
        .map(a => a.getAttribute("href"))
        .filter(href =>
            href &&
            href !== "../" &&
            !href.startsWith("?") &&
            !href.startsWith("#")
        )
        .map(name => folder + name);
}

function newLogElement(msg, color) {
    const logsElement = document.getElementById("logs");
    const newElement = document.createElement("div");
    const text = document.createElement("p");
    text.innerText = msg;
    text.classList.add(color);
    text.classList.add("m-0");
    text.classList.add("fs-6");
    text.classList.add("text-wrap");
    newElement.classList = "w-100 border-secondary border-bottom";
    newElement.appendChild(text);
    logsElement.appendChild(newElement);
    logsElement.scrollTop = logsElement.scrollHeight;
    return newElement;
}

function logInfo(...args) {
    const msg = args.join(" ");
    newLogElement(msg, "text-info");
}

function logError(...args) {
    const msg = args.join(" ");
    newLogElement(msg, "text-danger");
}

function prettifyUrl(url) {
    const fileName = url.split("/").pop();
    const decoded = decodeURIComponent(fileName);
    return decoded.replace(/\.[^/.]+$/, "");
}

function setLoggers() {

    const debugLogger = rmLines.addFunction(ptr => {
        const msg = rmLines.UTF8ToString(ptr);
        newLogElement(msg, "text-warning");
    }, "vi");

    const errorLogger = rmLines.addFunction(ptr => {
        const msg = rmLines.UTF8ToString(ptr);
        newLogElement(msg, "text-danger")
    }, "vi");

    rmLines.ccall(
        "setDebugLogger",
        null,
        ["number"],
        [debugLogger]
    );

    rmLines.ccall(
        "setErrorLogger",
        null,
        ["number"],
        [errorLogger]
    );
}

async function showModal(file) {
    const viewerContainer = document.getElementById("openseadragon1");
    const widthPx = window.innerWidth * 0.6;
    const heightPx = window.innerHeight * 0.95;
    viewerContainer.style.width = widthPx + "px";
    viewerContainer.style.height = heightPx + "px";

    if (tile_source) {
        tile_source.destroy();
        viewer.destroy();
    }

    tile_source = createTileSource(file)
    if (!await tile_source.init()) {
        logError("An issue loading the viewer occured at the initialization level!");
        return;
    }
    viewer = OpenSeadragon({
        id: "openseadragon1",
        prefixUrl: "https://cdn.jsdelivr.net/npm/openseadragon@5.0.1/build/openseadragon/images/",
        tileSources: tile_source,
        showNavigator: true,
    });

    const modal = new bootstrap.Modal(document.getElementById("imgModal"));
    modal.show();
}

function addImage(src, file) {
    const col = document.createElement("div");
    col.className = "col-4 col-sm-3 col-md-2 col-lg-1 p-1";

    const img = document.createElement("img");
    img.src = src;

    img.className = "img-fluid rounded shadow-sm w-100";
    img.style.cursor = "pointer";
    img.style.objectFit = "cover";

    img.onclick = () => showModal(file);

    col.appendChild(img);
    document.getElementById("grid").appendChild(col);
}

function createImage(image, paper_size, file) {
    const canvas = document.createElement("canvas");
    canvas.width = paper_size[0];
    canvas.height = paper_size[1];

    const ctx = canvas.getContext("2d");

    ctx.putImageData(image, 0, 0);

    canvas.toBlob((blob) => {
        addImage(URL.createObjectURL(blob), file);
    }, "image/png");
}

function fileId(path) {
    let hash = 5381;

    for (let i = 0; i < path.length; i++) {
        hash = ((hash << 5) + hash) + path.charCodeAt(i);
    }

    return (hash >>> 0).toString(36); // compact string id
}

async function linesBuildTree(file) {
    const response = await fetch(file);
    const data = new Uint8Array(await response.arrayBuffer());
    const file_id = fileId(file)

    rmLines.FS_writeFile(`/${file_id}`, data);
    const tree_id = rmLines.ccall(
        "buildTree",
        "string",
        ["string"],
        [`/${file_id}`]
    );
    rmLines.FS_unlink(`/${file_id}`);
    return tree_id;
}

function linesDestroyTree(tree_id) {
    rmLines.ccall(
        "destroyTree",
        null,
        ["string"],
        [tree_id]
    )
}

function linesDestroyRenderer(renderer_id) {
    rmLines.ccall(
        "destroyRenderer",
        null,
        ["string"],
        [renderer_id]
    )
}

function linesMakeRenderer(tree_id, page_type, landscape) {
    return rmLines.ccall(
        "makeRenderer",
        "string",
        [
            "string",
            "number",
            "number"
        ],
        [
            tree_id,
            page_type,
            landscape
        ],
    )
}

function linesGetSceneInfo(tree_id) {
    const sceneInfoRaw = rmLines.ccall(
        "getSceneInfo",
        "string",
        ["string"],
        [tree_id]
    )
    if (!sceneInfoRaw) {
        return null;
    }
    return JSON.parse(sceneInfoRaw);
}

function linesGetLayers(renderer_id) {
    const layersRaw = rmLines.ccall(
        "getLayers",
        "string",
        ["string"],
        [renderer_id]
    )
    if (!layersRaw) {
        return null;
    }
    return JSON.parse(layersRaw);
}

function linesGetSizeTracker(renderer_id, layer) {
    const groupId = layer.groupId;
    const trackerRaw = rmLines.ccall(
        "getSizeTracker",
        "string",
        ["string", "string"],
        [renderer_id, groupId]
    )
    if (!trackerRaw) {
        return null;
    }
    return JSON.parse(trackerRaw);
}

async function linesGetFrame(renderer_id, fX, fY, fW, fH, w, h, antialias) {
    // return queueTile(() => {
    const bufferSize = w * h;
    const byteSize = bufferSize * 4;

    // allocate uint32 buffer in WASM heap
    const bufferPtr = rmLines._malloc(byteSize);

    rmLines.ccall(
        "getFrame",
        null,
        [
            "string",  // renderer_id
            "number",  // uint32_t* buffer
            "number",  // size_t
            "number",  // x
            "number",  // y
            "number",  // fw
            "number",  // fh
            "number",  // w
            "number",  // h
            "number"   // antialias
        ],
        [
            renderer_id,
            bufferPtr,
            byteSize,
            fX,
            fY,
            fW,
            fH,
            w,
            h,
            antialias ? 1 : 0
        ]
    );
    const view = rmLines.HEAPU8.subarray(bufferPtr, bufferPtr + byteSize);
    const clamped = new Uint8ClampedArray(byteSize);
    clamped.set(view);
    const image = new ImageData(clamped, w, h)
    rmLines._free(bufferPtr)
    return image;
    // });
}

function getMaxBounds(renderer_id, paper_size) {
    const layers = linesGetLayers(renderer_id);
    if (!layers) {
        return [0, 0, paper_size[0], paper_size[1]];
    }
    let l = 0;
    let t = 0;
    let r = paper_size[0];
    let b = paper_size[1];
    for (const layer of layers) {
        const tracker = linesGetSizeTracker(renderer_id, layer);
        if (tracker) {
            l = Math.min(l, tracker.l);
            t = Math.min(t, tracker.t);
            r = Math.max(r, tracker.r);
            b = Math.max(b, tracker.b);
        }
    }
    return [l, t, r, b]
}

async function run() {
    rmLines = await RMLines();
    setLoggers();

    const all = [];
    for (const folder of folders) {
        const files = await getFiles(folder);
        all.push(...files);
    }

    for (const file of all) {
        logInfo("Reading", prettifyUrl(file));
        const tree_id = await linesBuildTree(file)

        if (!tree_id) {
            logError("Failed to build tree for", prettifyUrl(file));
            break;
        }

        // Read scene info
        const sceneInfo = linesGetSceneInfo(tree_id)
        let paper_size = [1404, 1872];
        if (sceneInfo && sceneInfo.paper_size) {
            paper_size = sceneInfo.paper_size;
        }

        const renderer_id = linesMakeRenderer(tree_id, 1, false)
        if (!renderer_id) {
            logError("Failed to build tree for", prettifyUrl(file));
            break;
        }

        // logInfo(getMaxBounds(renderer_id, paper_size))


        const image = await linesGetFrame(
            renderer_id, 0, 0, paper_size[0], paper_size[1], paper_size[0], paper_size[1], false
        )
        createImage(image, paper_size, file);

        linesDestroyRenderer(renderer_id);
        linesDestroyTree(tree_id);
    }
    logInfo("COMPLETED!");
}

run();